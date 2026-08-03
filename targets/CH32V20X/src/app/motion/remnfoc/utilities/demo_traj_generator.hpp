#pragma once

#include "../motor_prelude.hpp"
#include "mc_math.hpp"

namespace ymd::remnfoc{


enum class [[nodiscard]] DemoTrajPattern:uint8_t{
    Stop,
    Straight,
    LargeSine,
    Sine,
    Saw,
    Stairs,
    Triangle,
    Miniwave,
    ScanSine,
    ScanSquare,
};

template<size_t Q, typename D>
static constexpr math::fixed<Q, D> squpu(const math::fixed<Q, D> x){
    constexpr D MASK = D(1ull << (Q - 1));
    const D ret_bits = bool(x.to_bits() & MASK) << Q;
    return math::fixed<Q, D>::from_bits(ret_bits);
}

static_assert(squpu(0.2_iq16) == 0);
static_assert(squpu(0.7_iq16).to_bits() == 1 << 16);

__no_inline static constexpr TrajState
calc_demo_traj(const uq16 t, const DemoTrajPattern demo_pattern){
    switch(demo_pattern){
        case DemoTrajPattern::Stop:{
            return {0, 0, 0};
        }
        case DemoTrajPattern::Straight:{
            constexpr auto speed = 0.03_iq16;

            return {
                make_iiq32(speed * t),
                iq20(speed),
                0
            };
        }

        case DemoTrajPattern::Triangle:{
            static constexpr auto abs_delta = iq32(1.0 / 65536 / 16);
            static iiq32 position = 0;
            static constexpr int flip_duration = 32;
            const bool is_forward = int(t) % (flip_duration * 2) < flip_duration;

            constexpr iq32 abs_speed = iq32(abs_delta * CONF_FOC_FREQ);
            [[maybe_unused]] constexpr float abs_speed_f = (float)abs_speed;

            auto speed = abs_speed;
            auto delta = abs_delta;
            if(not is_forward){
                speed = -speed;
                delta = -delta;
            }

            position += iiq32::from_bits(delta.to_bits());
            return {
                position,
                iq20(speed),
                0
            };
        }
        case DemoTrajPattern::Sine:{
            constexpr auto speed = 1_iq16;
            constexpr auto side_amplitude = 1.4_iq16;

            const auto [s,c] = math::sincos(speed * t);
            return {
                make_iiq32(side_amplitude * iq16(s)),
                side_amplitude * speed * iq16(c),
                0
            };
        }

        case DemoTrajPattern::Miniwave:{
            constexpr auto speed = 4_iq16;
            constexpr auto side_amplitude = 0.00026_iq16;
            // constexpr auto side_amplitude = 0.006_iq16;

            const auto [s,c] = math::sincos(speed * t);
            return {
                make_iiq32(side_amplitude * iq16(s)),
                side_amplitude * speed * iq16(c),
                0
            };
        }

        case DemoTrajPattern::LargeSine:{
            constexpr auto speed = 1_iq16;
            constexpr auto side_amplitude = 4.00016_iq16;

            const auto [s,c] = math::sincos(speed * t);
            return {
                make_iiq32(side_amplitude * iq16(s)),
                side_amplitude * speed * iq16(c),
                0
            };
        }

        case DemoTrajPattern::ScanSquare:{
            // constexpr auto speed = 4_iq16;
            // constexpr auto side_amplitude = 0.0016_iq16;

            constexpr auto freq = 10;
            // constexpr auto freq = 16;
            // constexpr auto side_amplitude = 0.016_iq16;
            constexpr auto side_amplitude = 0.032_iq16;

            const auto p = bool(squpu(freq * t)) * side_amplitude;

            return {
                make_iiq32(p),
                0,
                0
            };
        }

        case DemoTrajPattern::ScanSine:{
            // constexpr auto speed = 4_iq16;
            // constexpr auto side_amplitude = 0.0016_iq16;

            // constexpr auto speed = 48_iq16;
            constexpr auto speed = 38_iq16;
            constexpr auto side_amplitude = 0.026_iq16;
            // constexpr auto side_amplitude = 0.006_iq16;

            const auto [s,c] = math::sincos(speed * t);
            return {
                make_iiq32(side_amplitude * iq16(s)),
                side_amplitude * speed * iq16(c),
                0
            };
        }
        case DemoTrajPattern::Saw:{
            // const auto [s,c] = math::sincos(speed * t);

            constexpr auto freq = 0.2_iq16;
            constexpr auto amplitude = 5.0_iq16;
            constexpr auto slew_rate = amplitude * freq;
            return {make_iiq32(-iq16(math::frac(t * freq)) * amplitude), -slew_rate, 0};
        }
        case DemoTrajPattern::Stairs:{
            constexpr auto freq = 0.3_iq16;
            constexpr size_t num_steps = 6;
            // constexpr auto half_amplitude = 0.4_iq16;
            // constexpr auto half_amplitude = 0.04_iq16;
            constexpr auto half_amplitude = 0.004_iq16;
            constexpr auto step = half_amplitude * 2/ num_steps;
            const auto s = iq16(math::sinpu(t * freq));
            return {
                make_iiq32((math::floor(s * (num_steps / 2)) * step)), 
                0,
                0
            };
        }
    }
    //unreachable
    return {0, 0, 0};
};


}