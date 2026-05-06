#pragma once

#include <bitset>
#include <type_traits>

#include "core/math/fixed/fixed.hpp"
#include "primitive/arithmetic/angular.hpp"


namespace ymd::robots::dji::c620{


namespace utils{

template<size_t Q>
[[nodiscard]] static constexpr int16_t scale_16384_by_20(math::fixed<Q, int32_t> x){
    using T = math::fixed<Q, int32_t>;
    constexpr float MAX_VALUE = 20.0f - 1e-7f;
    constexpr float MIN_VALUE = -MAX_VALUE;
    x = std::clamp(x, T(MIN_VALUE), T(MAX_VALUE));

    constexpr int32_t FACTOR = (16384.0 / 20.0f) * (1u << (32 - x.NUM_Q));
    const int32_t ret_bits = int32_t((static_cast<int64_t>(x.to_bits()) * FACTOR) >> 32);

    if(ret_bits > (int32_t)std::numeric_limits<int16_t>::max()) __builtin_unreachable();
    if(ret_bits < (int32_t)std::numeric_limits<int16_t>::min()) __builtin_unreachable();
    return static_cast<int16_t>(ret_bits);
}


template<std::floating_point T>
[[nodiscard]] static constexpr int16_t scale_16384_by_20(T x){
    constexpr float FACTOR = 16384.0 / 20.0f;
    constexpr float MAX_VALUE = 20.0f - 1e-7f;
    constexpr float MIN_VALUE = -MAX_VALUE;
    x = std::clamp(x, MIN_VALUE, MAX_VALUE);
    int32_t bits = static_cast<int32_t>(x * FACTOR);
    if(bits > (int32_t)std::numeric_limits<int16_t>::max()) __builtin_unreachable();
    if(bits < (int32_t)std::numeric_limits<int16_t>::min()) __builtin_unreachable();
    return static_cast<int16_t>(bits);
}

template<typename T>
[[nodiscard]] static constexpr int16_t scale_16384_by_10(T x){
    return scale_16384_by_20(x * 2);
}


template<typename T>
struct scale_1_by_60;

template<typename T>
requires(std::is_floating_point_v<T>)
struct scale_1_by_60<T>{
    [[nodiscard]] static constexpr T calc(int16_t x){
    constexpr float FACTOR = 1.0f / 60;
    return (x * FACTOR);
    }
};

template<size_t Q>
struct scale_1_by_60<math::fixed<Q, int32_t>>{
    using T = math::fixed<Q, int32_t>;

    [[nodiscard]] static constexpr T calc(int16_t x){
        const auto temp = math::fixed<Q, int32_t>(x) * uq32(1.0 / 60);
        return temp;
    }
};


template<typename T>
struct scale_20_by_16384;

template<typename T>
requires(std::is_floating_point_v<T>)
struct scale_20_by_16384<T>{
    [[nodiscard]] static constexpr T calc(int16_t x){
        constexpr float FACTOR = 20.0f / 16384;
        return static_cast<float>(x) * FACTOR;
    }
};

template<size_t Q>
struct scale_20_by_16384<math::fixed<Q, int32_t>>{
    using T = math::fixed<Q, int32_t>;
    static constexpr int32_t FACTOR = int32_t(20.0f / 16384 * (1ull << 32));

    [[nodiscard]] static constexpr T calc(int16_t x){
        return T::from_bits((int64_t(x) * FACTOR) >> (32 - Q));
    }
};


}


};