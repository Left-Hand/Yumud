#pragma once

#include "core/math/realmath.hpp"

namespace ymd::remnfoc{

template<size_t Q, typename D>
requires(sizeof(D) == 4)
static constexpr iiq32 make_iiq32(const math::fixed<Q, D> x){
    const D bits = x.to_bits();
    constexpr size_t LEFT_SHIFTS = 32 - Q;
    return iiq32::from_bits(int64_t(bits) << LEFT_SHIFTS);
} 

static constexpr uq32 iiq32_crop_frac(const iiq32 x){
    return uq32::from_bits(uint32_t(x.to_bits() & UINT32_MAX));
}

static constexpr int32_t iiq32_crop_revs(const iiq32 x){
    return int32_t(x.to_bits() >> 32);
}


static constexpr iiq32 iiq32_add_revs(const iiq32 x, const int32_t n_revs){
    const auto frac = iiq32_crop_frac(x);
    const int32_t revs = iiq32_crop_revs(x);
    return iiq32::from_bits(int64_t(int64_t(revs + n_revs) << 32) | frac.to_bits());
}


// static_assert(iiq32_add_revs(iiq32(0.2f), 1).to_bits() == iiq32(1.2f).to_bits());

template<size_t Q_final, typename D_final, size_t Q, typename D>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q_final, D_final> 
sub_clamp2_downcast(math::fixed<Q, D> lhs, math::fixed<Q, D> rhs, math::fixed<Q_final, D_final> ma){
    constexpr size_t RIGHT_SHIFTS = Q - Q_final;
    D y_bits = D((lhs.to_bits() - rhs.to_bits()));
    const D ma_bits = D(ma.to_bits()) << RIGHT_SHIFTS;
    if(y_bits > ma_bits) y_bits = ma_bits;
    if(y_bits < -ma_bits) y_bits = -ma_bits;
    return math::fixed<Q_final, D_final>::from_bits(y_bits >> RIGHT_SHIFTS);
}

static_assert(sub_clamp2_downcast(iiq32(1000), iiq32(-200), iq20(1000)).to_bits() == iq20(1000).to_bits());
static_assert(sub_clamp2_downcast(iiq32(6000), iiq32(-800), iq20(1000)).to_bits() == iq20(1000).to_bits());
static_assert(sub_clamp2_downcast(iiq32(-6000), iiq32(-800), iq20(1000)).to_bits() == iq20(-1000).to_bits());
static_assert(sub_clamp2_downcast(iiq32(0), iiq32(-800), iq20(1000)).to_bits() == iq20(800).to_bits());



__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<20, int32_t> 
sub_clamp2_downcast_iq20_roundlsb(iiq32 lhs, iiq32 rhs, int32_t ma){
    #if 1
    constexpr size_t RIGHT_SHIFTS = (32 - 20) - 1;

    int64_t y_bits = int64_t((lhs.to_bits() - rhs.to_bits()));
    const int32_t head_bits = int32_t(y_bits >> 32);

    if(head_bits > ma) return iq20(ma);
    if(head_bits < -ma) return iq20(-ma);
    int32_t final_bits = int32_t(y_bits >> RIGHT_SHIFTS);
    return iq20::from_bits(int32_t(final_bits + 1) >> 1);
    #else
    return sub_clamp2_downcast<20, int32_t>(lhs, rhs, iq20(ma));
    #endif
}

static_assert(sub_clamp2_downcast_iq20_roundlsb(iiq32(1000), iiq32(-200), (1000)).to_bits() == iq20(1000).to_bits());
static_assert(sub_clamp2_downcast_iq20_roundlsb(iiq32(6000), iiq32(-800), (1000)).to_bits() == iq20(1000).to_bits());
static_assert(sub_clamp2_downcast_iq20_roundlsb(iiq32(-6000), iiq32(-800), (1000)).to_bits() == iq20(-1000).to_bits());
// static_assert(sub_clamp2_downcast_iq20_roundlsb(iiq32(3.5), iiq32(1.2), (1000)).to_bits() == iq20(2.3).to_bits());
// static_assert(sub_clamp2_downcast_iq20_roundlsb(iiq32(13.5), iiq32(1.2), (1000)).to_bits() == iq20(12.3).to_bits());
static_assert(sub_clamp2_downcast_iq20_roundlsb(iiq32(0), iiq32(-800), (1000)).to_bits() == iq20(800).to_bits());



static constexpr size_t pow2(const size_t x){
    return __builtin_ctz(x);
}

static_assert(pow2(1) == 0);
static_assert(pow2(2) == 1);
static_assert(pow2(16) == 4);

static constexpr std::tuple<uq32, int32_t> iiq32_depart(const iiq32 x){
    const uq32 prev_lap = uq32::from_bits(uint32_t(x.to_bits() & UINT32_MAX));
    int32_t floor_turns = int32_t(x.to_bits() >> 32);

    return {prev_lap, floor_turns};
} 


static constexpr iiq32 uq32_wrapped_diff(const uq32 last, const uq32 now){
    const iiq32 diff = iiq32::from_bits(
        static_cast<int64_t>(now.to_bits()) - static_cast<int64_t>(last.to_bits())
    );
    if(diff > iiq32(0.5)) return diff - 1;
    if(diff < iiq32(-0.5)) return diff + 1;
    return diff;
};

static_assert(uq32_wrapped_diff(0.35_uq32, 0.6_uq32).to_bits() == iiq32(0.25).to_bits());
static_assert(uq32_wrapped_diff(0.75_uq32, 0.50_uq32).to_bits() == iiq32(-0.25).to_bits());
static_assert(uq32_wrapped_diff(0.05_uq32, 0.80_uq32).to_bits() == iiq32(-0.25).to_bits());

static constexpr iiq32 iiq32_inc_uq32_wrapped(const iiq32 state, const uq32 last, const uq32 now){
    const auto diff = uq32_wrapped_diff(last, now);
    return state + diff;
}

// __no_inline
static constexpr iiq32 uq32_wrapped_update(
    iiq32 prev_absolute_position, 
    uq32 lap_position
){
    #if 0
    auto [prev_lap, floor_turns] = iiq32_depart(prev_absolute_position);
    const uint32_t lap_u32 = lap_position.to_bits();
    const uint32_t prev_lap_u32 = prev_lap.to_bits();

    if(lap_u32 - prev_lap_u32 > uint32_t(0x8000'0000)) [[unlikely]]{ 
        floor_turns--;
    }else if(prev_lap_u32 - lap_u32 > uint32_t(0x8000'0000)) [[unlikely]]{
        floor_turns++;
    }

    return iiq32::from_bits(int64_t(floor_turns) << 32 | lap_u32);
    #else
    // iiq32 diff = iiq32::from_bits(
    //     static_cast<int64_t>(lap_position.to_bits()) - static_cast<int64_t>(prev_absolute_position.to_bits() & UINT32_MAX)
    // );
    // if(diff > iiq32(0.5)) diff -= 1;
    // if(diff < iiq32(-0.5)) return diff += 1;
    // return prev_absolute_position + diff;

    return iiq32_inc_uq32_wrapped(
        prev_absolute_position, 
        uq32::from_bits(uint32_t(prev_absolute_position.to_bits() & UINT32_MAX)),
        lap_position);
    #endif
}


}