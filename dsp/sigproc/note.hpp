#pragma once

#include <cmath>
#include <numbers>
#include <cstdint>

namespace dsp {

template<typename T>
struct Note {
    static constexpr T kA4 = T(440.0f);
    static constexpr int32_t kA4Midi = 69;

    static constexpr T kA0 = T(27.5f);
    static constexpr int32_t kA0Midi = 21;

    static T midi_2_freq(T midi) {
        static constexpr auto inv_12 = T(1.0f / 12.0f);
        return kA4 * std::exp2((midi - kA4Midi) );
    }


    static constexpr T pi = std::numbers::pi_v<T>;
    static constexpr T twopi = 2.0f * pi;
};

} // namespace dsp
