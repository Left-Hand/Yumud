#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

#include "core/math/fixed/fixed.hpp"

namespace ymd::drivers{


struct TemperatureCode{
    std::array<uint8_t, 2> bytes;

    static constexpr TemperatureCode from_b11(uint16_t v){
        v = v << 5;
        TemperatureCode self;
        self.bytes[0] = uint8_t(v & 0xff);
        self.bytes[1] = uint8_t((v & 0xff00) >> 8);
        return self;
    }


    constexpr int16_t i11() const {
        int16_t b = 0;
        b |= (bytes[1] << 8);
        b |= (bytes[0] << 0);
        b = b >> 5;
        return b;
    }

    constexpr iq16 to_celsius() const {
        return i11() * iq16(0.125);
    }
};

struct R8_Config{
    uint8_t shutdown_en:1;
    uint8_t os_comp_int:1;
    uint8_t os_pol:1;
    uint8_t os_f_que:1;
    uint8_t __resv__:2;
};

static_assert(TemperatureCode::from_b11(0x7ff).i11() == -1);
static_assert(TemperatureCode::from_b11(0x3f8).i11() == 1016);
static_assert(TemperatureCode::from_b11(0x3f8).to_celsius() == 127);
static_assert(TemperatureCode::from_b11(0x648).to_celsius() == -55);

struct LM75_Prelude{



};


}