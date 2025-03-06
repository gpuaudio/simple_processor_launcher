#include <gtest/gtest.h>

#include "GainSpecification.h"
#include "TestCommon.h"

#include <GPUCreate.h>

namespace {
void apply_gain(TestData& data, float gain) {
    for (uint32_t ch {0u}; ch < data.m_nchannels; ++ch) {
        for (uint32_t s {0u}; s < data.m_nsamples; ++s) {
            data.at(ch, s) *= gain;
        }
    }
}
} // namespace

TEST(ProcLaunchLib, CreateDestroy) {
    std::unique_ptr<ProcessorLauncherInterface> ProcLaunchLib = createGpuProcessorLauncher(2u, 256u);
    ASSERT_NE(ProcLaunchLib, nullptr);
}
