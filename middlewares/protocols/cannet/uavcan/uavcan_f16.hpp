#pragma once

#include <cstdint>
// #include "core/math/float/fp32.hpp"

namespace ymd::uavcan{

namespace details{

union _FP32{
    uint32_t u;
    float f;
};
constexpr union _FP32 f32inf = { 255UL << 23 };
constexpr union _FP32 f16inf = { 31UL << 23 };

constexpr uint32_t sign_mask = 0x80000000U;
constexpr uint32_t round_mask = ~0xFFFU;
static constexpr uint16_t ConvertFloatToFloat16(float value){
    constexpr union _FP32 magic = {15UL<< 23};
    _FP32 in;
    in.f = value;
    uint32_t sign = in.u & sign_mask;
    in.u ^= sign;
    uint16_t out = 0;
    if (in.u >= f32inf.u){
        out =(in.u > f32inf.u) ?(uint16_t)0x7FFFU : (uint16_t)0x7C00U;
    }else{
        in.u &= round_mask;
        in.f *= magic.f;
        in.u-= round_mask;
        if(in.u >f16inf.u){
            in.u = f16inf.u;
        }
        out =(uint16_t)(in.u >>13U);
    }
    out |= (uint16_t)(sign >> 16U);
    return out;
}

static constexpr float ConvertFloat16ToFloat(uint16_t value){
    constexpr union _FP32 magic ={ (254UL-15UL) << 23U };
    constexpr union _FP32 was_inf_nan = { (127UL + 16UL) << 23U };
    union _FP32 out;
    out.u =(value & 0x7FFFU) << 13U;
    out.f *= magic.f;
    if (out.f >= was_inf_nan.f){
        out.u |= 255UL <<23U;
    }
    out.u |= (value & 0x8000UL) << 16U;
    return out.f;
}
}



struct [[nodiscard]] f16 final{
    using Self = f16;

    uint16_t bits;

    [[nodiscard]] constexpr f16(const float f){
        bits = details::ConvertFloatToFloat16(f);
    }

    [[nodiscard]] constexpr explicit operator float() const{
        return details::ConvertFloat16ToFloat(bits);
    }

    [[nodiscard]] constexpr uint16_t to_bits() const{
        return bits;
    }

    [[nodiscard]] static constexpr Self from_bits(const uint16_t bits) {
        return std::bit_cast<Self>(bits);
    }
};


}