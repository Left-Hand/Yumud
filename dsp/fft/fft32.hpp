#pragma once

#include "core/math/real.hpp"
#include "core/math/iq/iqmath.hpp"
#include <span>

namespace ymd::dsp{


static constexpr auto SINCOS32_1_TABLE = []{
    constexpr size_t N = 32;
    std::array<std::array<math::fixed_t<15, int16_t>, 2>, 32> table;
    uq16 x = 0;
    constexpr uq16 step = uq16::from_rcp(N);
    for(size_t i = 0; i < N; i++){
        const auto [s,c] = math::sincospu(x);
        table[i] = std::to_array({
            math::fixed_t<15, int16_t>::from_bits(iq15(s).to_bits()), 
            math::fixed_t<15, int16_t>::from_bits(iq15(c).to_bits())
        });
        x += step;
    }
    return table;
}();

static constexpr auto SINCOS32_2_TABLE = []{
    constexpr size_t N = 32;
    std::array<std::array<math::fixed_t<15, int16_t>, 2>, 32> table;
    uq16 x = 0;
    constexpr uq16 step = uq16::from_rcp(N) * 2;
    for(size_t i = 0; i < N; i++){
        const auto [s,c] = math::sincospu(x);
        table[i] = std::to_array({
            math::fixed_t<15, int16_t>::from_bits(iq15(s).to_bits()), 
            math::fixed_t<15, int16_t>::from_bits(iq15(c).to_bits())
        });
        x += step;
    }
    return table;
}();


template<size_t Q>
__attribute__((optimize("O3"), hot, flatten))
constexpr std::tuple<math::fixed_t<Q, int32_t>, math::fixed_t<Q, int32_t>> dft32_bin2(
    std::span<const math::fixed_t<Q, int32_t>, 32> real_in
){
	int64_t real_bits = 0;
	int64_t imag_bits = 0;

    // 使用restrict指针
    const auto* __restrict in_ptr = real_in.data();
    const  std::array<math::fixed_t<15, int16_t>, 2> * __restrict tab_ptr = SINCOS32_2_TABLE.data();

    #pragma GCC unroll 4
	for (size_t i = 0; i < 32; i++) {
        const auto [s, c] = tab_ptr[i];
		real_bits += static_cast<int64_t>(in_ptr[i].to_bits()) * s.to_bits();
		imag_bits -= static_cast<int64_t>(in_ptr[i].to_bits()) * c.to_bits();
	}
    return {
        math::fixed_t<Q, int32_t>::from_bits(static_cast<int32_t>(real_bits >> (5 + 15))),
        math::fixed_t<Q, int32_t>::from_bits(static_cast<int32_t>(imag_bits >> (5 + 15))),
    };
}

template<size_t Q>
__attribute__((optimize("O3"), hot, flatten))
constexpr std::tuple<math::fixed_t<Q, int32_t>, math::fixed_t<Q, int32_t>> dft32_bin1(
    std::span<const math::fixed_t<Q, int32_t>, 32> real_in
){
	int64_t real_bits = 0;
	int64_t imag_bits = 0;

    // 使用restrict指针
    const auto* __restrict in_ptr = real_in.data();
    const std::array<math::fixed_t<15, int16_t>, 2> * __restrict tab_ptr = SINCOS32_1_TABLE.data();

    #pragma GCC unroll 4
	for (size_t i = 0; i < 32; i++) {
        const auto [s, c] = tab_ptr[i];
		real_bits += static_cast<int64_t>(in_ptr[i].to_bits()) * s.to_bits();
		imag_bits -= static_cast<int64_t>(in_ptr[i].to_bits()) * c.to_bits();
	}
    return {
        math::fixed_t<Q, int32_t>::from_bits(static_cast<int32_t>(real_bits >> (5 + 15))),
        math::fixed_t<Q, int32_t>::from_bits(static_cast<int32_t>(imag_bits >> (5 + 15))),
    };
}

template<size_t Q>
__attribute__((optimize("O3"), hot, flatten))
constexpr math::fixed_t<Q, int32_t> dft32_bin0(
    std::span<const math::fixed_t<Q, int32_t>, 32> real_in
){
	int64_t real_bits = 0;

    // 使用restrict指针
    const auto* __restrict in_ptr = real_in.data();

    #pragma GCC unroll 4
	for (size_t i = 0; i < 32; i++) {
		real_bits += static_cast<int64_t>(in_ptr[i].to_bits());
	}

    return  math::fixed_t<Q, int32_t>::from_bits(static_cast<int32_t>(real_bits >> (5)));
}

}