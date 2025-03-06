/*
 * Copyright (c) 2024 Braingines SA - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef FIR_FIR_SPECIFICATION_H
#define FIR_FIR_SPECIFICATION_H

#include <cstdint>
#include <stddef.h>

namespace FirConfig {

struct Parameters {
    static constexpr uint32_t Magic = 0xBB81EC22;
    uint32_t ThisMagic {Magic};

    uint32_t ir_index {};
};

struct Specification {
    static constexpr uint32_t Magic = 0xAC90FB31;
    uint32_t ThisMagic {Magic};

    uint32_t filter_length {121522u};
    uint32_t filter_index {121522u / 2u};
    uint32_t last_choice {0u};
};

} // namespace FirConfig

#endif // FIR_FIR_SPECIFICATION_H
