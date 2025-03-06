#ifndef GPUA_GPU_PROCESSOR_LAUNCHER_PROCESSOR_H
#define GPUA_GPU_PROCESSOR_LAUNCHER_PROCESSOR_H

#include <ProcessorLauncherInterface.h>

#include <engine_api/GraphLauncher.h>
#include <engine_api/Module.h>
#include <engine_api/ProcessingGraph.h>
#include <engine_api/Processor.h>

#include <gpu_audio_client/ProcessExecutorSync.h>

#include <array>
#include <cstdint>
#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <vector>

/**
 * The GPU processor launcher; implements the ProcessorLauncherInterface
 */
class GPUProcessorLauncher : public ProcessorLauncherInterface {
public:
    /**
     * @brief Constructor
     * @param nchannels [in] number of channels of the audio data to process
     * @param nsamples_per_channel [in] maximum number of samples per channel in the processing-buffer
     */
    GPUProcessorLauncher(uint32_t nchannels, uint32_t nsamples_per_channel);

    /**
     * @brief Destructor
     */
    virtual ~GPUProcessorLauncher();

    ////////////////////////////////
    // ProcessorLauncherInterface methods
    virtual void arm() override;
    virtual void disarm() override;
    virtual void process(float const* const* in_buffer, float* const* out_buffer, int nsamples) override;

    virtual void load_processor(wchar_t const* p_id, void const* p_data, uint32_t p_data_size) override;
    // ProcessorLauncherInterface methods
    ////////////////////////////////

private:
    std::mutex m_armed_mutex;
    bool m_armed {false};

    uint32_t const m_nchannels;
    static constexpr uint32_t MaxSampleCount {4096u};

    GPUA::engine::v2::GraphLauncher* m_launcher {nullptr};
    GPUA::engine::v2::ProcessingGraph* m_graph {nullptr};

    /**
     * Contains everything required to create/delete a processor instance
     */
    struct ProcDesc {
        GPUA::engine::v2::Module* m_module {nullptr};
        std::vector<std::byte> m_processor_spec;
        GPUA::engine::v2::Processor* m_processor {nullptr};
    };

    std::vector<ProcDesc> m_processors;

    ProcessExecutorConfig m_executor_config;
    ProcessExecutor<ExecutionMode::eSync>* m_process_executor {nullptr};
};

#endif // GPUA_GPU_PROCESSOR_LAUNCHER_PROCESSOR_H
