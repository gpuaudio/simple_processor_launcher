#include <GPUCreate.h>

#include "GPUProcessorLauncher.h"

std::unique_ptr<ProcessorLauncherInterface> createGpuProcessorLauncher(uint32_t nchannels, uint32_t nsamples_per_channel) {
    return std::make_unique<GPUProcessorLauncher>(nchannels, nsamples_per_channel);
}
