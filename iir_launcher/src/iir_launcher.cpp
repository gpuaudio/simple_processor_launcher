#include <AudioFile/AudioFile.h>
#include <GPUCreate.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

// include the processor specification; required to create an instance of the processor
#include <iir_processor/IirSpecification.h>

/**
 * Simple command line application to process a *.wav file with the iir processor
 */
int main(int argc, char** argv) {
    if (argc < 2 || (argc - 2) % 3) {
        printf("Error: usage iir_launcher.exe [[sr] [bp_f] [bp_q]]_1 ... <[sr] [bp_f] [bp_q]>_n [input.wav]\n");
        return 1;
    }

    // parse and process command line arguments
    uint32_t niirs = static_cast<uint32_t>((argc - 2) / 3);
    std::vector<float> params(niirs * 3u);
    std::string params_str {};
    for (uint32_t i {0u}; i < niirs; ++i) {
        params[3u * i + 0u] = static_cast<float>(std::atof(argv[3u * i + 1u]));
        params_str += "_" + std::to_string(params[3 * i + 0]);

        params[3u * i + 1u] = static_cast<float>(std::atof(argv[3u * i + 2u]));
        params_str += "_" + std::to_string(params[3 * i + 1]);

        params[3u * i + 2u] = static_cast<float>(std::atof(argv[3u * i + 3u]));
        params_str += "_" + std::to_string(params[3 * i + 2]);
    }

    std::string infilepath(argv[argc - 1]);

    std::string outputfile = std::filesystem::path(infilepath).filename().replace_extension().string() + "_iir" + params_str + ".wav";
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

    // load the IIR processor(s)
    for (uint32_t i {0u}; i < niirs; ++i) {
        IirConfig::Specification iir_spec {
            .sample_rate = params[3u * i + 0u],
            .band_pass_freq = params[3u * i + 1u],
            .band_pass_q = params[3u * i + 2u]};
        proc_launcher->load_processor(L"iir", &iir_spec, sizeof(iir_spec));
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

    printf("IIR processing was successful\n");

    return 0;
}
