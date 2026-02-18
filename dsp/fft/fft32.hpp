#pragma once

#include "core/math/real.hpp"
#include "core/math/fixed/iqmath.hpp"
#include <span>

namespace ymd::dsp{

template<size_t Q>
__attribute__((optimize("O3"), hot, flatten))
constexpr math::fixed<Q, int32_t> dft32_bin0(
    std::span<const math::fixed<Q, int32_t>, 32> real_in
){
    constexpr size_t N_BITS = 5;
    constexpr size_t N = 1u << N_BITS;
	int64_t real_bits = 0;

    // 使用restrict指针
    const auto* __restrict in_ptr = real_in.data();

    #pragma GCC unroll 4
	for (size_t i = 0; i < N; i++) {
		real_bits += static_cast<int64_t>(in_ptr[i].to_bits());
	}

    return  math::fixed<Q, int32_t>::from_bits(static_cast<int32_t>(real_bits >> (N_BITS)));
}

namespace {

template<size_t Q, typename T, size_t N, size_t Step>
static constexpr auto _make_sincos_table(){
    std::array<std::array<math::fixed<Q, T>, 2>, N> table;
    uq16 x = 0;
    constexpr uq16 step = uq16::from_rcp(N) * Step;
    for(size_t i = 0; i < N; i++){
        const auto [s,c] = math::sincospu(x);
        table[i] = std::to_array({
            math::fixed<Q, T>::from_bits(iq15(s).to_bits()), 
            math::fixed<Q, T>::from_bits(iq15(c).to_bits())
        });
        x += step;
    }
    return table;
};

template<size_t N_BITS, size_t BinNum, size_t Q>
__attribute__((optimize("O3"), hot, flatten))
constexpr std::tuple<math::fixed<Q, int32_t>, math::fixed<Q, int32_t>> _dft(
    std::span<const math::fixed<Q, int32_t>, (1u << N_BITS)> real_in
){
    constexpr size_t N = 1u << N_BITS;
    constexpr size_t TABLE_Q = 15;
    constexpr auto TABLE = _make_sincos_table<TABLE_Q, int16_t, N, BinNum>();

	int64_t real_bits = 0;
	int64_t imag_bits = 0;
    // 使用restrict指针
    const auto* __restrict in_ptr = real_in.data();
    const std::array<math::fixed<TABLE_Q, int16_t>, 2> * __restrict tab_ptr = TABLE.data();

    #pragma GCC unroll 4
	for (size_t i = 0; i < N; i++) {
        const auto [s, c] = tab_ptr[i];
		real_bits += static_cast<int64_t>(in_ptr[i].to_bits()) * s.to_bits();
		imag_bits -= static_cast<int64_t>(in_ptr[i].to_bits()) * c.to_bits();
	}
    return {
        math::fixed<Q, int32_t>::from_bits(static_cast<int32_t>(real_bits >> (N_BITS + TABLE_Q))),
        math::fixed<Q, int32_t>::from_bits(static_cast<int32_t>(imag_bits >> (N_BITS + TABLE_Q))),
    };
}


}

static constexpr auto SINCOS32_1_TABLE = _make_sincos_table<15, int16_t, 32, 1>();
static constexpr auto SINCOS32_2_TABLE = _make_sincos_table<15, int16_t, 32, 2>();

template<size_t Q>
__attribute__((optimize("O3"), hot, flatten))
constexpr std::tuple<math::fixed<Q, int32_t>, math::fixed<Q, int32_t>> dft32_bin1(
    std::span<const math::fixed<Q, int32_t>, 32> real_in
){
    return _dft<5, 1, Q>(real_in);
}



template<size_t Q>
__attribute__((optimize("O3"), hot, flatten))
constexpr std::tuple<math::fixed<Q, int32_t>, math::fixed<Q, int32_t>> dft32_bin2(
    std::span<const math::fixed<Q, int32_t>, 32> real_in
){
    return _dft<5, 2, Q>(real_in);
}

template<size_t N, typename TABLE_T>
struct DftCalcultor{
    static_assert(std::is_signed_v<TABLE_T>);
    static_assert(std::has_single_bit(N));

    static constexpr size_t N_BITS = std::countr_zero(N);
    static constexpr size_t TABLE_Q = sizeof(TABLE_T) * 8 - 1;
    static constexpr size_t IDX_MASK = N - 1;
    static constexpr auto BIN1_TABLE = _make_sincos_table<15, int16_t, N, 1>();

    template<size_t Q, typename D>
    static constexpr std::array<math::fixed<Q, D>, 2> calc_dft(
        size_t bin_num,
        std::span<const math::fixed<Q, D>, N> real_in
    ){ 
        int64_t real_bits = 0;
        int64_t imag_bits = 0;
        // 使用restrict指针
        const auto* __restrict in_ptr = real_in.data();
        const auto * __restrict bin1_table_ptr = BIN1_TABLE.data();

        size_t i = 0;
        #pragma GCC unroll 4
        for (size_t _ = 0; _ < N; i++) {
            const auto [s, c] = bin1_table_ptr[i];
            real_bits += static_cast<int64_t>(in_ptr[i].to_bits()) * s.to_bits();
            imag_bits -= static_cast<int64_t>(in_ptr[i].to_bits()) * c.to_bits();
            i = (i + bin_num) & IDX_MASK;
        }

        return {
            math::fixed<Q, int32_t>::from_bits(static_cast<int32_t>(real_bits >> (N_BITS + TABLE_Q))),
            math::fixed<Q, int32_t>::from_bits(static_cast<int32_t>(imag_bits >> (N_BITS + TABLE_Q))),
        };
    }

    template<size_t NUM_BINS, size_t Q, typename D>
    static constexpr std::array<std::array<math::fixed<Q, D>, 2>, NUM_BINS> calc_dft(
        std::array<size_t, NUM_BINS> bins,
        std::span<const math::fixed<Q, D>, N> real_in
    ){ 
        std::array<std::array<math::fixed<Q, D>, 2>, NUM_BINS> ret;
        for(size_t i = 0; i < NUM_BINS; i++){
            ret[i] = calc_dft(bins[i], real_in);
        }
    }
};


}