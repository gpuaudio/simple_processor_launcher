#pragma once

#include "ProcessorLauncherInterface.h"

#include <cstdint>
#include <memory>

/**
 * @brief Create an instance of the GPUProcessorLauncher.
 * @param nchannels [in] number of channels of the audio data to process
 * @param nsamples_per_channel [in] capacity of the processing-buffer per channel
 * @return ProcessorLauncherInterface pointer to the created GPUProcessorLauncher instance
 */
std::unique_ptr<ProcessorLauncherInterface> createGpuProcessorLauncher(uint32_t nchannels = 2u, uint32_t nsamples_per_channel = 256u);
