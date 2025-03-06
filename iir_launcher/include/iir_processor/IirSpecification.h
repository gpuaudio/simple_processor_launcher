/*
 * Copyright (c) 2024 Braingines SA - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef IIR_IIR_SPECIFICATION_H
#define IIR_IIR_SPECIFICATION_H

#include <cstdint>
#include <stddef.h>

namespace IirConfig {

struct Parameters {
    static constexpr uint32_t Magic = 0xCF104BC;
    uint32_t ThisMagic {Magic};
};

struct Specification {
    static constexpr uint32_t Magic = 0xCF104BD;
    uint32_t ThisMagic {Magic};

    float sample_rate {96000.f};
    float band_pass_freq {5000.f};
    float band_pass_q {0.5f};
};

} // namespace IirConfig

#endif // IIR_IIR_SPECIFICATION_H
