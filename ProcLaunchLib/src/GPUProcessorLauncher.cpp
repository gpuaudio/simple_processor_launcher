#include "GPUProcessorLauncher.h"

#include <gpu_audio_client/GpuAudioManager.h>

#include <engine_api/DeviceInfoProvider.h>
#include <engine_api/LauncherSpecification.h>
#include <engine_api/ModuleInfo.h>

#define _USE_MATH_DEFINES
#include <algorithm>
#include <array>
#include <cassert>
#include <math.h>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>

GPUProcessorLauncher::GPUProcessorLauncher(uint32_t nchannels, uint32_t nsamples_per_channel) :
    m_nchannels {nchannels} {
    // buffer settings and double buffering configuration (see `gpu_audio_client` for details)
    m_executor_config = {
        .retain_threshold = 0.625,
        .launch_threshold = 0.7275,
        .nchannels_in = m_nchannels,
        .nchannels_out = m_nchannels,
        .max_samples_per_channel = nsamples_per_channel};

    // create gpu_audio engine and make sure a supported GPU is installed/selected
    const auto& gpu_audio = GpuAudioManager::GetGpuAudio();
    const auto& device_info_provider = gpu_audio->GetDeviceInfoProvider();
    const auto dev_idx = GpuAudioManager::GetDeviceIndex();
    if (dev_idx >= device_info_provider.GetDeviceCount()) {
        throw std::runtime_error("No supported device found");
    }
    // get all the information about the GPU device required to create a launcher
    GPUA::engine::v2::LauncherSpecification launcher_spec = {};
    if ((device_info_provider.GetDeviceInfo(dev_idx, launcher_spec.device_info) != GPUA::engine::v2::ErrorCode::eSuccess) || !launcher_spec.device_info) {
        throw std::runtime_error("Failed to get device info");
    }

    // create a launcher for the specified GPU device
    if ((gpu_audio->CreateLauncher(launcher_spec, m_launcher) != GPUA::engine::v2::ErrorCode::eSuccess) || !m_launcher) {
        throw std::runtime_error("Failed to create launcher");
    }

    // create a processing graph to create the processor(s) in
    if ((m_launcher->CreateProcessingGraph(m_graph) != GPUA::engine::v2::ErrorCode::eSuccess) || !m_graph) {
        gpu_audio->DeleteLauncher(m_launcher);
        throw std::runtime_error("Failed to create processing graph");
    }
};

GPUProcessorLauncher::~GPUProcessorLauncher() {
    // delete executor and processor
    disarm();
    // delete the processing graph
    m_launcher->DeleteProcessingGraph(m_graph);
    // delete the launcher
    GpuAudioManager::GetGpuAudio()->DeleteLauncher(m_launcher);
}

void GPUProcessorLauncher::arm() {
    std::lock_guard<std::mutex> lock(m_armed_mutex);

    if (!m_armed) {
        thread_local std::vector<GPUA::engine::v2::Processor*> processors;
        processors.clear();
        for (auto& p_desc : m_processors) {
            // use the processor descriptor to create the specified processor in the graph
            if (p_desc.m_module->CreateProcessor(m_graph, p_desc.m_processor_spec.data(), p_desc.m_processor_spec.size(), p_desc.m_processor) != GPUA::engine::v2::ErrorCode::eSuccess || !p_desc.m_processor) {
                throw std::runtime_error("Failed to create processor");
            }
            // connect each but the first processor's input to the previous processor's output
            if (!processors.empty()) {
                if (p_desc.m_processor->SetInputByPortId(0u, processors.back()->GetOutputByPortId(0u)) != GPUA::engine::v2::ErrorCode::eSuccess) {
                    throw std::runtime_error("Failed to connect processors");
                }
            }
            processors.emplace_back(p_desc.m_processor);
        }
        // create an executor that manages input and output buffers and performs the actual launches
        m_process_executor = new ProcessExecutor<ExecutionMode::eSync>(m_launcher, m_graph, static_cast<uint32_t>(processors.size()), processors.data(), m_executor_config);
        m_armed = true;
    }
}

void GPUProcessorLauncher::disarm() {
    std::lock_guard<std::mutex> lock(m_armed_mutex);

    if (m_armed) {
        // delete the executor. ensures that all launches have finished before destroying itself.
        if (m_process_executor) {
            delete m_process_executor;
            m_process_executor = nullptr;
        }

        // as no more launches are active, it's safe to destroy the processor
        for (auto& p_desc : m_processors) {
            p_desc.m_module->DeleteProcessor(p_desc.m_processor);
            p_desc.m_processor = nullptr;
        }
    }
    m_armed = false;
}

void GPUProcessorLauncher::process(float const* const* in_buffer, float* const* out_buffer, int nsamples) {
    // If the GPUProcessorLauncher was not armed ahead of time, arm it on the first process call.
    if (!m_armed) {
        arm();
    }

    thread_local std::vector<float const*> input_ptrs;
    thread_local std::vector<float*> output_ptrs;
    // If we have to perform multiple launches, we have to copy the channel pointers s.t. we can advance them.
    if (nsamples > m_executor_config.max_samples_per_channel) {
        input_ptrs.assign(in_buffer, in_buffer + m_nchannels);
        output_ptrs.assign(out_buffer, out_buffer + m_nchannels);
    }

    uint32_t remaining_samples = nsamples;
    while (remaining_samples != 0) {
        // determine the number of samples for this launch
        uint32_t this_launch_samples = std::min(m_executor_config.max_samples_per_channel, remaining_samples);
        // process samples [i, i + this_launch_samples)
        m_process_executor->template Execute<AudioDataLayout::eChannelsIndividual>(this_launch_samples, in_buffer, out_buffer);

        // advance channel pointers for the next iteration if required
        remaining_samples -= this_launch_samples;
        if (remaining_samples != 0) {
            for (auto& ptr : input_ptrs)
                ptr += this_launch_samples;
            in_buffer = input_ptrs.data();

            for (auto& ptr : output_ptrs)
                ptr += this_launch_samples;
            out_buffer = output_ptrs.data();
        }
    }
}

void GPUProcessorLauncher::load_processor(wchar_t const* p_id, void const* p_data, uint32_t p_data_size) {
    std::lock_guard<std::mutex> lock(m_armed_mutex);
    if (m_armed) {
        throw std::runtime_error("Error GPUProcessorLauncher::load_processor called while armed");
    }

    // add a descriptor for the processor that's to be loaded
    ProcDesc& p_desc = m_processors.emplace_back();

    // get the module provider from the launcher to access all available modules (read as processors here)
    auto& module_provider = m_launcher->GetModuleProvider();
    const auto module_count = module_provider.GetModulesCount();
    GPUA::engine::v2::ModuleInfo info {};
    bool processor_module_found = false;
    // iterate the module infos and try to find the requested processor by matching the id
    for (uint32_t i = 0; i < module_count; ++i) {
        if ((module_provider.GetModuleInfo(i, info) == GPUA::engine::v2::ErrorCode::eSuccess) && info.id && (std::wcscmp(info.id, p_id) == 0)) {
            processor_module_found = true;
            break;
        }
    }
    // we could not find the processor
    if (!processor_module_found) {
        m_processors.pop_back();
        throw std::runtime_error("Failed to find required processor module");
    }

    // get the processor's module; we need this to create and destroy the processor instance
    if ((module_provider.GetModule(info, p_desc.m_module) != GPUA::engine::v2::ErrorCode::eSuccess) || !p_desc.m_module) {
        m_processors.pop_back();
        throw std::runtime_error("Failed to load required processor module");
    }

    // create a local copy of the provided specification to guarantee it is still available when we (re-)create the processor
    std::byte const* p_data_bytes = reinterpret_cast<std::byte const*>(p_data);
    p_desc.m_processor_spec.assign(p_data_bytes, p_data_bytes + p_data_size);
}
