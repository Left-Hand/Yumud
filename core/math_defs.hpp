#pragma once

#include <type_traits>
#include <bits/move.h>
#include <bit>

#include "string.h"

namespace ymd{

#ifndef LN2
#define LN2 0.6931471805599453094172321215
#endif

#ifndef TAU
#define TAU 6.2831853071795864769252867666
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433833
#endif

#ifndef M_E
#define M_E 2.7182818284590452353602874714
#endif

#ifndef HUGE_VALL
#define HUGE_VALL (__builtin_huge_vall())
#endif

#ifndef INFINITY
#define INFINITY (__builtin_inff())
#endif

#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif


#ifndef SQRT3
#define SQRT3 1.73205080757f
#endif

#ifndef SQRT2
#define SQRT2 1.41421356237f
#endif

#ifndef LOG_E
#define LOG_E 0.434294481903
#endif

#ifndef LOG_2
#define LOG_2 0.301029995664
#endif


namespace details{
template<typename First>
__attribute__((always_inline)) constexpr First 
__max_helper(const First & value) {
    return value;
}

template<typename First, typename Second, typename... Rest>
__attribute__((always_inline)) constexpr First 
__max_helper(const First & first,const Second & second,Rest && ... rest){
    First max_value = first > First(second) ? first : First(second);
    return __max_helper(max_value, rest...);
}

template<typename First>
__attribute__((always_inline)) constexpr First 
__min_helper(const First & value) {
    return value;
}

template<typename First, typename Second, typename... Rest>
__attribute__((always_inline)) constexpr First 
__min_helper(const First & first,const Second & second,Rest &&... rest) {
    First min_value = first < First(second) ? first : First(second);
    return __min_helper(min_value, rest...);
}

template<typename T>
__attribute__((always_inline)) constexpr T 
__clamp_helper(const T x, const auto mi, const auto ma) {
    if((x > static_cast<T>(ma))) [[unlikely]]
        return static_cast<T>(ma);
    if((x < static_cast<T>(mi))) [[unlikely]]
        return static_cast<T>(mi);
    return x;
}


}

template<typename ... Ts >
[[nodiscard]] __attribute__((always_inline)) constexpr 
auto MAX(Ts && ... args){return details::__max_helper(std::forward<Ts>(args)...);}

template<typename ... Ts >
[[nodiscard]] __attribute__((always_inline)) constexpr 
auto MIN(Ts && ... args){return details::__min_helper(std::forward<Ts>(args)...);}


template <typename T>
requires std::is_arithmetic_v<T>
[[nodiscard]] __attribute__((always_inline)) constexpr T ABS(const T a){
    return (a < 0) ? -a : a;
}



template <typename T>
requires std::is_arithmetic_v<T>
[[nodiscard]] __attribute__((always_inline)) constexpr T SIGN(const T a){
    return (a < 0) ? T(-1) : T(1);
}

[[nodiscard]] __attribute__((always_inline)) constexpr 
bool IN_RANGE(auto x,auto a,auto b) {return((a < b) ? (x >= a && x < b) : (x < a && x >= b));}


template <typename T, typename U>
requires std::is_arithmetic_v<U>
[[nodiscard]] __attribute__((always_inline)) constexpr T LERP(const T & a, const T & b, const U & x){
    return T((U(U(1) - x) * a) + (x * b));
}




template <typename T, typename U, typename V>
[[nodiscard]] __attribute__((always_inline)) constexpr T INVLERP(const U & _a, const V & _b, const T& t){
    T a = static_cast<T>(_a);
    T b = static_cast<T>(_b);
    return (t - a) / (b - a);
}



[[nodiscard]] __attribute__((always_inline)) constexpr 
auto CLAMP(auto x,auto mi,auto ma){return details::__clamp_helper(x, mi, ma);}

[[nodiscard]] __attribute__((always_inline)) constexpr 
auto CLAMP2(auto x, auto ma){ return details::__clamp_helper(x, -ma, ma);}


template <typename T>
requires std::is_arithmetic_v<T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T STEP_TO(const T x,const T y, const T s){
    T err = y-x;
    if(err > s){
        return x + s;
    }else if(err < -s){
        return x - s;
    }else{
        return y;
    }
}


template<typename T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T LSHIFT(const T x, const int s){
    if (s >= 0){
        return x << s;
    }else{
        return x >> -s;
    }
}


template<typename T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T RSHIFT(const T x, const int s){
    if (s >= 0){
        return x >> s;
    }else{
        return x << -s;
    }
}

[[nodiscard]] __attribute__((always_inline)) constexpr 
uint32_t NEXT_POWER_OF_2(const uint32_t x) {
    return ((x == 0) ? 1 : (1 << (32 - __builtin_clz(x - 1))));
}

[[nodiscard]] __attribute__((always_inline)) constexpr 
uint32_t PREV_POWER_OF_2(const uint32_t x) {
    return (1 << (31 - __builtin_clz(x)));
}


#ifndef CTZ
#define CTZ(x) __builtin_ctz((size_t)(x))
#endif


#ifndef CLZ
#define CLZ(x) __builtin_clz(((size_t)(x)) << (PLAT_WIDTH - BITS(x)))
#endif


static constexpr double DEG2RAD_RATIO = (TAU / 360);
static constexpr double RAD2DEG_RATIO = (360 / TAU);



}