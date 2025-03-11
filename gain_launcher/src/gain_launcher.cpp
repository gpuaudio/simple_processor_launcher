#include <AudioFile/AudioFile.h>
#include <GPUCreate.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

// include the processor specification; required to create an instance of the processor
#include <gain_processor/GainSpecification.h>

/**
 * Simple command line application to process a *.wav file with the gain processor
 */
int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Error: usage gain_launcher.exe [gain_1]..<gain_n> [input.wav]\n");
        return 1;
    }

    // parse and process command line arguments
    uint32_t ngains = static_cast<uint32_t>(argc - 2);
    std::vector<float> gains(ngains);
    std::string gains_str {};
    for (uint32_t g_id = 0; g_id < ngains; ++g_id) {
        gains[g_id] = static_cast<float>(std::atof(argv[1 + g_id]));
        gains_str += "_" + std::to_string(gains[g_id]);
    }

    std::string infilepath(argv[argc - 1]);

    std::string outputfile = std::filesystem::path(infilepath).filename().replace_extension().string() + "_gain" + gains_str + ".wav";
    std::string outfilepath = (std::filesystem::path(infilepath).parent_path() / outputfile).string();

    // load input wav
    AudioFile<float> input;
    if (!input.load(infilepath)) {
        printf("Could not open input from %s\n", infilepath.c_str());
        return 1;
    }
    uint32_t nchannels = input.getNumChannels();

    // create processor launcher with buffer_size samples per process buffer
    constexpr uint32_t buffer_size {512u};
    auto proc_launcher = createGpuProcessorLauncher(nchannels, buffer_size);
    if (proc_launcher == nullptr) {
        printf("Could not create processor launcher\n");
        return 2;
    }

    // load the gain processor(s)
    for (uint32_t g_id {0u}; g_id < ngains; ++g_id) {
        GainConfig::Specification gain_spec {.params {.gain_value = gains[g_id]}};
        proc_launcher->load_processor(L"gain", &gain_spec, sizeof(gain_spec));
    }

    AudioFile<float> output {input};
    uint32_t const nsamples_total = input.getNumSamplesPerChannel();
    std::vector<float*> in_ptr(nchannels, nullptr), out_ptr(nchannels, nullptr);

    // call process with `buffer_size`-sized chunks of the input (causing one internal process call at a time)
    for (uint32_t cursor {0u}; cursor < nsamples_total; cursor += buffer_size) {
        for (uint32_t ch {0u}; ch < nchannels; ++ch) {
            in_ptr[ch] = input.samples[ch].data() + cursor;
            out_ptr[ch] = output.samples[ch].data() + cursor;
        }
        uint32_t nsamples = std::min<uint32_t>(buffer_size, nsamples_total - cursor);
        proc_launcher->process(in_ptr.data(), out_ptr.data(), nsamples);
    }

    // write the output
    if (!output.save(outfilepath)) {
        printf("Could not save output to %s\n", outfilepath.c_str());
        return 3;
    }

    printf("Gain processing was successful\n");

    return 0;
}
