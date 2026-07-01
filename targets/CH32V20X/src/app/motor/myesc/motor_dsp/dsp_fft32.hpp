#pragma once

#include "core/math/real.hpp"
#include "core/math/fixed/fxmath.hpp"
#include <span>

namespace ymd::dsp{

template<size_t Q>
__attribute__((optimize("Ofast"), hot, flatten))
constexpr math::fixed<Q, int32_t> dft32_bin0(
    std::span<const math::fixed<Q, int32_t>, 32> real_in
){
    constexpr size_t N_BITS = 5;
    constexpr size_t N = 1u << N_BITS;
	int32_t real_bits = 0;

    // 使用restrict指针
    const auto* __restrict in_ptr = real_in.data();

    #pragma GCC unroll 4
	for (size_t i = 0; i < N; i++) {
		real_bits += static_cast<int32_t>(in_ptr[i].to_bits());
	}

    return  math::fixed<Q, int32_t>::from_bits(static_cast<int32_t>(real_bits >> (N_BITS)));
}

namespace {

template<size_t Q, typename T, size_t N, size_t Step>
static constexpr auto _make_sincos_table(){
    std::array<std::array<math::fixed<Q, T>, 2>, N> table;
    uq32 x = 0;
    constexpr uq32 delta = uq32::from_rcp(N) * Step;
    for(size_t i = 0; i < N; i++){
        const auto [s,c] = math::sincospu(x);
        table[i] = std::to_array({
            math::fixed<Q, T>::from_bits(iq15(s).to_bits()), 
            math::fixed<Q, T>::from_bits(iq15(c).to_bits())
        });
        x += delta;
    }
    return table;
};

static constexpr auto DFT32_BIN1_SINCOS_TABLE = _make_sincos_table<15, int16_t, 32, 1>();

template<size_t N_BITS, size_t BinNum, size_t Q>
__attribute__((optimize("Ofast"), hot, flatten))
constexpr std::tuple<math::fixed<Q, int32_t>, math::fixed<Q, int32_t>> _dft(
    std::span<const math::fixed<Q, int32_t>, (1u << N_BITS)> real_in
){
    constexpr size_t N = 1u << N_BITS;
    constexpr size_t TABLE_Q = 15;


	int64_t real_bits = 0;
	int64_t imag_bits = 0;
    // 使用restrict指针
    const math::fixed<Q, int32_t> * __restrict in_ptr = real_in.data();
    const std::array<math::fixed<TABLE_Q, int16_t>, 2> * __restrict tab_ptr = DFT32_BIN1_SINCOS_TABLE.data();


    #pragma GCC unroll 4
	for (size_t i = 0; i < N; i++) {
        const auto [s, c] = tab_ptr[(i * BinNum) & (N - 1)];
		real_bits += static_cast<int64_t>(in_ptr[i].to_bits()) * static_cast<int32_t>(c.to_bits());
		imag_bits -= static_cast<int64_t>(in_ptr[i].to_bits()) * static_cast<int32_t>(s.to_bits());
	}

    return {
        math::fixed<Q, int32_t>::from_bits(static_cast<int32_t>(real_bits >> (N_BITS + TABLE_Q))),
        math::fixed<Q, int32_t>::from_bits(static_cast<int32_t>(imag_bits >> (N_BITS + TABLE_Q))),
    };
}


}



template<size_t Q>
__attribute__((optimize("Ofast"), hot, flatten))
constexpr std::tuple<math::fixed<Q, int32_t>, math::fixed<Q, int32_t>> dft32_bin1(
    std::span<const math::fixed<Q, int32_t>, 32> real_in
){
    return _dft<5, 1, Q>(real_in);
}



template<size_t Q>
__attribute__((optimize("Ofast"), hot, flatten))
constexpr std::tuple<math::fixed<Q, int32_t>, math::fixed<Q, int32_t>> dft32_bin2(
    std::span<const math::fixed<Q, int32_t>, 32> real_in
){
    return _dft<5, 2, Q>(real_in);
}


}