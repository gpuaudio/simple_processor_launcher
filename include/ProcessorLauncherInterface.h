#ifndef PROCESSOR_LAUNCHER_INTERFACE_H
#define PROCESSOR_LAUNCHER_INTERFACE_H

#include <cstdint>

/**
 * Public interface for the processor launcher library
 */
class ProcessorLauncherInterface {
public:
    /**
     * @brief Default destructor
     */
    virtual ~ProcessorLauncherInterface() = default;

    /**
     * @brief Process samples provided in input and write them to output buffers.
     * @param input [in] pointer to pointers to the channels of the input audio data
     * @param output [in/out] pointer to pointers to the channels of the output audio data
     */
    virtual void process(float const* const* input, float* const* output, const int nsamples) = 0;

    /**
     * @brief Get the client library ready for processing with the current configuration
     */
    virtual void arm() = 0;

    /**
     * @brief Clean up and get ready for destruction or re-configuration
     */
    virtual void disarm() = 0;

    /**
     * @brief Load a processor into the launcher
     * @param p_id [in] Unique identifier of the processor to load; see processor's ModuleInfoProvider
     * @param p_data [in] Pointer to the processor's specification; see the processor's constructor
     * @param p_data_size [in] Size of p_data in bytes
     */
    virtual void load_processor(wchar_t const* p_id, void const* p_data, uint32_t p_data_size) = 0;
};

#endif // PROCESSOR_LAUNCHER_INTERFACE_H
