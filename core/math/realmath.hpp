#pragma once

#include "real.hpp"
#include "core/math_defs.hpp"

#include <cmath>
#include <numbers>

#include "fixed/fxmath.hpp"

namespace ymd::math{

    
template<typename T>
requires (std::is_arithmetic_v<T>)
constexpr T powfi(const T base, const size_t exponent) {
    if(0 == exponent) {
        return T(1);
    }else if(1 == exponent){
        return base;
    }else{
        T ret = 0;
        if(1 < exponent){
            for(size_t i = 1; i < exponent; ++i){
                ret = ret * base;
            }
        }else{
            for(size_t i = 1; i < -exponent; ++i){
                ret = ret / base;
            }
        }
        return ret;
    }
}

template<std::floating_point T>
constexpr T sinpu(const T val){
    constexpr T tau = static_cast<T>(M_TWOPI);
    return std::sin(static_cast<T>(val * tau));
}

template<std::floating_point T>
constexpr T cospu(const T val){
    constexpr T tau = static_cast<T>(M_TWOPI);
    return std::cos(static_cast<T>(val * tau));
}


template<typename T>
requires (std::is_arithmetic_v<T>)
constexpr T square(const T x) {
    return x * x;
}


template<typename T>
requires (std::is_arithmetic_v<T>)
constexpr T cubic(const T x) {
    return x * x * x;
}

template<typename T>
requires (std::is_arithmetic_v<T>)
constexpr T distance(const T a, const T b){
    return ABS(a-b);
}

template<typename T>
requires (std::is_arithmetic_v<T>)
constexpr T normal(const T a, const T b){
    return SIGN(b - a);
}


template<std::floating_point T>
constexpr T inv_sqrt(const T val){
    return 1 / std::sqrt(val);
}



template<std::floating_point T, typename... Args>
constexpr T mag(const T first, const Args... rest) {
    if constexpr (sizeof...(rest) == 0) {
        return std::abs(first);
    } else {
        return std::sqrt(square(first) + (square(rest) + ...));
    }
}

template<std::floating_point T, typename... Args>
constexpr T inv_mag(const T first, const Args... rest) {
    return 1 / mag(first, rest...);
}

template<std::floating_point T>
constexpr std::array<T, 2> math::sincospu(const T turns){
    const auto radians = turns * static_cast<T>(TAU);
    return {std::sin(radians), std::cos(radians)};
}

template<std::floating_point T>
constexpr std::array<T, 2> sincos(const T radians){
    return {std::sin(radians), std::cos(radians)};
}

template<std::floating_point T>
constexpr T atan2pu(const T a, const T b) {
    constexpr auto INV_TAU = static_cast<T>(1 / TAU);
    return std::atan2(a, b) * INV_TAU;
}

template<std::floating_point T>
constexpr T floor(const T x) {
    return std::floor(x);
}

template<typename T>
requires (std::is_floating_point_v<T>)
[[nodiscard]] static constexpr T 
atan2(T y, T x){
    return std::atan2(y, x);
}
template<typename T>
requires (std::is_floating_point_v<T>)
[[nodiscard]] static constexpr T 
clamp(T x, T min, T max){
    if(x > max) [[unlikely]] return max;
    if(x < min) [[unlikely]] return min;
    return x;
}

template<typename T>
requires (std::is_floating_point_v<T>)
[[nodiscard]] static constexpr T 
asin(T x){
    return std::asin(x);
}

template<typename T>
requires (std::is_floating_point_v<T>)
[[nodiscard]] static constexpr T 
acos(T x){
    return std::acos(x);
}

template<typename T>
requires (std::is_floating_point_v<T>)
[[nodiscard]] static constexpr T 
cos(T x){
    return std::cos(x);
}

template<typename T>
requires (std::is_floating_point_v<T>)
[[nodiscard]] static constexpr T 
sin(T x){
    return std::sin(x);
}

template<typename T>
requires (std::is_floating_point_v<T>)
[[nodiscard]] static constexpr T 
sqrt(T x){
    return std::sqrt(x);
}
}
