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


static constexpr auto SINCOS_32STEP_TABLE = []{
    using T = int32_t;

    static constexpr size_t Q = 15;
    static constexpr size_t NUM_STEPS = 32;
    std::array<std::array<math::fixed<15, T>, 2>, NUM_STEPS> table;
    uq32 x = 0;
    constexpr uq32 delta = uq32::from_rcp(NUM_STEPS);
    for(size_t i = 0; i < NUM_STEPS; i++){
        const auto [s,c] = math::sincospu(x);
        table[i] = std::to_array({
            math::fixed<Q, T>::from_bits(math::fixed<Q, T>(s).to_bits()), 
            math::fixed<Q, T>::from_bits(math::fixed<Q, T>(c).to_bits())
        });
        x += delta;
    }
    return table;
}();

static constexpr std::array<iq15, 2> sincos_32step(const size_t idx){
    return SINCOS_32STEP_TABLE[idx];
}


namespace {




template<size_t BinNum, size_t Q>
__attribute__((optimize("Ofast"), hot, flatten))
constexpr std::tuple<math::fixed<Q, int32_t>, math::fixed<Q, int32_t>> _dft(
    std::span<const math::fixed<Q, int32_t>, 32> real_in
){
    constexpr size_t N = 32;
    constexpr size_t N_BITS  = 5;
    constexpr size_t TABLE_Q = 15;


	int64_t real_bits = 0;
	int64_t imag_bits = 0;
    // 使用restrict指针
    const math::fixed<Q, int32_t> * __restrict in_ptr = real_in.data();
    const auto * __restrict tab_ptr = SINCOS_32STEP_TABLE.data();


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
    return _dft<1, Q>(real_in);
}



template<size_t Q>
__attribute__((optimize("Ofast"), hot, flatten))
constexpr std::tuple<math::fixed<Q, int32_t>, math::fixed<Q, int32_t>> dft32_bin2(
    std::span<const math::fixed<Q, int32_t>, 32> real_in
){
    return _dft<2, Q>(real_in);
}


}