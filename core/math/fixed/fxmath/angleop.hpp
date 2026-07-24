#pragma once

#include "port.hpp"
#include <cstddef>


#ifndef M_PI
#define M_PI (3.1415926536)
#endif


namespace ymd::math{


template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((const, always_inline)) static constexpr 
fixed<32, uint32_t> pu_to_uq32(const fixed<Q, D> x){
    if constexpr(std::is_signed_v<D>){
        return fixed<32, uint32_t>::from_bits(std::bit_cast<uint32_t>(x.to_bits()) << (32 - Q));
    } else {
        return fixed<32, uint32_t>::from_bits(x.to_bits() << (32 - Q));
    }
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((const, always_inline)) static constexpr 
fixed<32, uint32_t> rad_to_uq32(const fixed<Q, D> x){
    constexpr uint64_t uq32_inv_tau_bits = static_cast<uint64_t>(static_cast<long double>(
        static_cast<uint64_t>(1u) << (32)) / static_cast<long double>(M_PI * 2));

    auto conv_positive = [&]{
        return fixed<32, uint32_t>::from_bits(static_cast<uint32_t>((x.to_bits() * uq32_inv_tau_bits) >> Q));
    };

    auto conv_negative = [&]{
        return fixed<32, uint32_t>::from_bits(~static_cast<uint32_t>((static_cast<uint32_t>(-(x.to_bits())) * uq32_inv_tau_bits) >> Q));
    };

    if constexpr(std::is_signed_v<D>){
        if(x >= 0) return conv_positive();
        return conv_negative();
    } else {
        return conv_positive();
    }
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((const, always_inline)) static constexpr 
fixed<32, uint32_t> deg_to_uq32(const fixed<Q, D> x){
    constexpr uint64_t uq32_inv_tau_bits = static_cast<uint64_t>(static_cast<long double>(
        static_cast<uint64_t>(1u) << (32)) / static_cast<long double>(180 * 2));

    auto conv_positive = [&]{
        return fixed<32, uint32_t>::from_bits(static_cast<uint32_t>((x.to_bits() * uq32_inv_tau_bits) >> Q));
    };

    auto conv_negative = [&]{
        return fixed<32, uint32_t>::from_bits(~static_cast<uint32_t>((static_cast<uint32_t>(-(x.to_bits())) * uq32_inv_tau_bits) >> Q));
    };

    if constexpr(std::is_signed_v<D>){
        if(x >= 0) return conv_positive();
        return conv_negative();
    } else {
        return conv_positive();
    }
}

__attribute__((const, always_inline)) static constexpr 
fixed<29, int32_t> uq32_to_rad(const fixed<32, uint32_t> x){

    constexpr uint64_t uq29_tau_bits = static_cast<uint64_t>(static_cast<long double>(
        static_cast<uint64_t>(1u) << (29)) * static_cast<long double>(M_PI * 2));

    return fixed<29, int32_t>::from_bits(
        (static_cast<uint64_t>(std::bit_cast<int32_t>(x.to_bits())) * uq29_tau_bits) >> 32);
}



}