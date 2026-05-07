#pragma once

#include <bitset>
#include <type_traits>

#include "core/math/fixed/fixed.hpp"
#include "primitive/arithmetic/angular.hpp"


namespace ymd::robots::dji::gm6020{


namespace utils{

template<size_t Q>
[[nodiscard]] static constexpr int16_t scale_16384_by_3(math::fixed<Q, int32_t> x){
    using T = math::fixed<Q, int32_t>;
    constexpr float MAX_VALUE = 3.0f - 1e-7f;
    constexpr float MIN_VALUE = -MAX_VALUE;
    x = std::clamp(x, T(MIN_VALUE), T(MAX_VALUE));

    constexpr int32_t FACTOR = (16384.0f / 3) * (1u << (32 - x.NUM_Q));
    int32_t ret_bits = int32_t((static_cast<int64_t>(x.to_bits()) * FACTOR) >> 32);

    ret_bits = std::clamp(ret_bits, int32_t(-16384), int32_t(16384));
    return static_cast<int16_t>(ret_bits);
}


template<std::floating_point T>
[[nodiscard]] static constexpr int16_t scale_16384_by_3(T x){
    constexpr float FACTOR = 16384.0f;
    constexpr float MAX_VALUE = 3.0f - 1e-7f;
    constexpr float MIN_VALUE = -MAX_VALUE;
    x = std::clamp(x, MIN_VALUE, MAX_VALUE);
    int32_t ret_bits = static_cast<int32_t>(x * FACTOR);

    ret_bits = std::clamp(ret_bits, int32_t(-16384), int32_t(16384));
    return static_cast<int16_t>(ret_bits);
}

template<typename T>
[[nodiscard]] static constexpr int16_t scale_16384_by_3(T x){
    return scale_16384_by_3(x * 2);
}

template<typename T>
struct scale_3_by_16384;

template<typename T>
requires(std::is_floating_point_v<T>)
struct scale_3_by_16384<T>{
    [[nodiscard]] static constexpr T calc(int16_t x){
        constexpr float FACTOR = 3.0f / 16384;
        const int32_t bits = int32_t(x * FACTOR);
        return static_cast<int16_t>(bits);
    }
};

template<size_t Q>
struct scale_3_by_16384<math::fixed<Q, int32_t>>{
    using T = math::fixed<Q, int32_t>;

    [[nodiscard]] static constexpr T calc(int16_t x){
        constexpr float FACTOR = 3.0f / 16384;
        const auto temp = math::fixed<Q, int32_t>(x) * uq32(FACTOR);
        return temp;
    }
};

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

}


};