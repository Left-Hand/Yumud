#pragma once

#include "sys/core/sys_defs.h"

#ifdef __cplusplus
#include <type_traits>
#include "bits/move.h"
#include <bit>
#endif

#define CMP_EPSILON 0.001
#define CMP_EPSILON2 (CMP_EPSILON * CMP_EPSILON)

#define CMP_NORMALIZE_TOLERANCE 0.001
#define CMP_POINT_IN_PLANE_EPSILON 0.001

#ifndef LN2
#define LN2 0.6931471805599453094172321215
#endif

#ifndef TAU
#define TAU 6.2831853071795864769252867666
#endif

#ifndef PI
#define PI 3.1415926535897932384626433833
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


#ifndef MAX
#ifdef __cplusplus
    #define MAX(a,...) __max_helper(a,__VA_ARGS__)

    template<typename First>
    constexpr First __max_helper(const First & value) {
        return value;
    }

    template<typename First, typename Second, typename... Rest>
    constexpr First __max_helper(const First & first,const Second & second,const Rest & ... rest){
        First max_value = first > First(second) ? first : First(second);
        return __max_helper(max_value, rest...);
    }
#else
    #define MAX(x,y) ((x > y) ? x : y)
#endif
#endif

#ifndef MIN
#ifdef __cplusplus
    #define MIN(a,...) __min_helper(a,__VA_ARGS__)


    template<typename First>
    constexpr First __min_helper(const First & value) {
        return value;
    }

    template<typename First, typename Second, typename... Rest>
    constexpr First __min_helper(const First & first,const Second & second,const Rest &... rest) {
        First min_value = first < First(second) ? first : First(second);
        return __min_helper(min_value, rest...);
    }
#else
    #define MIN(x,y) ((x < y) ? x : y)
#endif
#endif

#ifndef ABS
#ifdef __cplusplus
    #define ABS(a) __abs_tmpl(a)
    template <typename T>
    requires std::is_arithmetic_v<T>
    constexpr __fast_inline T __abs_tmpl(const T a){
        return (a < 0) ? -a : a;
    }
#else
#define ABS(x) ((x < 0)? -(x) : x)
#endif
#endif



#ifndef SIGN
#define SIGN(x) ((x < 0) ? -1 : 1)
#endif

#ifndef TYPE_CHECK
#define TYPE_CHECK(a,b) (typeof(a) == typeof(b))
#endif

#ifndef IN_RANGE
#define IN_RANGE(x,a,b) ((a < b) ? (x >= a && x < b) : (x < a && x >= b))
#endif

#ifndef SWAP
#ifdef __cplusplus
    #define SWAP(m_x, m_y) std::swap((m_x), (m_y))
    // template <class T>
    // constexpr __fast_inline void __swap_tmpl(T &x, T &y) {
    //     T aux = x;
    //     x = y;
    //     y = aux;
    // }
    
#else
    #define SWAP(x,y)   do{typeof(x) t;t=x;x=y;y=t;}while(0)
#endif
#endif

#ifndef LERP
#ifdef __cplusplus
    #define LERP(a,b,x) __lerp_tmpl(a, b, x)
    template <typename T, typename U>
    requires std::is_arithmetic_v<U>
    constexpr __fast_inline T __lerp_tmpl(const T & a, const T & b, const U & x){
        return T((U(U(1) - x) * a) + (x * b));
    }
#else
#define LERP(a,b, x) (a * (1 - x) + b * x)
#endif
#endif

#ifndef INVLERP
#ifdef __cplusplus
    #define INVLERP(x,a,b) __invlerp_tmpl(x,a,b)
    template <typename T, typename U, typename V>
    constexpr __fast_inline T __invlerp_tmpl(const T& t, const U & _a, const V & _b){
        T a = static_cast<T>(_a);
        T b = static_cast<T>(_b);
        return (t - a) / (b - a);
    }
#else
    #define INVLERP(x,a,b) ((x - a) / (b - a))
#endif
#endif

#ifndef CLAMP
#if defined(__cplusplus)
    #define CLAMP(x, mi, ma) __clamp_tmpl(x, mi, ma)
    #define CLAMP2(x, ma) __clamp_tmpl(x, -ma, ma)

    template<typename T>
    constexpr __fast_inline T __clamp_tmpl(const T x, const auto mi, const auto ma) {
        if(unlikely(x > static_cast<T>(ma))) return static_cast<T>(ma);
        if(unlikely(x < static_cast<T>(mi))) return static_cast<T>(mi);
        return x;
    }
#else
    #define CLAMP(x, mi, ma) MIN(MAX(x, mi), ma)
    #define CLAMP2(x, ma) CLAMP(x, -ma, ma)
#endif
#endif

#ifndef STEP_TO
#ifdef __cplusplus
    #define STEP_TO(x, y, s) __step_tmpl(x, y, s)
    template <typename T>
    requires std::is_arithmetic_v<T>
    constexpr __fast_inline T __step_tmpl(const T x,const T y, const T s){
        T err = y-x;
        if(err > s){
            return x + s;
        }else if(err < -s){
            return x - s;
        }else{
            return y;
        }
    }
#else
#define STEP_TO(x, y, s) CLAMP(y, x - s, x + s)
#endif
#endif


#ifndef SIGN_AS
#ifdef __cplusplus
#define SIGN_AS(x,s) __sign_as_impl(x, s)

template<typename T>
constexpr __fast_inline T __sign_as_impl(const T x, const auto s){
    if(s){
        return s > 0 ? x : -x;
    }else{
        return T(0);
    }
}
#endif
#endif

#ifndef SIGN_DIFF
#ifdef __cplusplus
#define SIGN_DIFF(x,y) __sign_diff_impl(x, y)

template<typename T>
requires (sizeof(T) <= 4) 
constexpr __fast_inline bool __sign_diff_impl(const T x, const auto y){
    return (std::bit_cast<uint32_t>(x) ^ std::bit_cast<uint32_t>(y)) & 0x80000000;
}

#endif
#endif

#ifndef SIGN_SAME
#ifdef __cplusplus
#define SIGN_SAME(x,y) __sign_same_impl(x, y)

template<typename T>
requires (sizeof(T) <= 4) 
constexpr __fast_inline bool __sign_same_impl(const T x, const auto y){
    return (std::bit_cast<uint32_t>(x) ^ std::bit_cast<uint32_t>(y)) == 0;
}

#endif
#endif

#ifndef INVERSE_IF
#ifdef __cplusplus
#define INVERSE_IF(b, x) __inverse_if_impl(b, x)

template<typename T>
constexpr __fast_inline T __inverse_if_impl(const bool b, const T & x){
    return b ? -x : x;
}

#endif
#endif

#ifndef LSHIFT
#ifdef __cplusplus
#define LSHIFT(x,s) __lshift_impl(x, s);
template<typename T>
constexpr __fast_inline T __lshift_impl(const T & x, const int s){
    if (s >= 0){
        return x << s;
    }else{
        return x >> -s;
    }
}
#else
#define LSHIFT(x,s) ((s) >= 0 ? ((x) << (s)) : ((x) >> (-(s))))
#endif

#endif

#ifndef RSHIFT
#define RSHIFT(x,s) LSHIFT(x, (-s))
#endif

#define NEXT_POWER_OF_2(x) ((x == 0) ? 1 : (1 << (32 - __builtin_clz(x - 1))))
#define PREV_POWER_OF_2(x) (1 << (31 - __builtin_clz(x)))

#define CTZ(x) __builtin_ctz((size_t)(x))
#define BITS(x) (sizeof(x) * 8)
#define PLAT_WIDTH (BITS(size_t))
#define CLZ(x) __builtin_clz(((size_t)(x)) << (PLAT_WIDTH - BITS(x)))

#define ANGLE2RAD(x) ((x) * TAU / 360)
#define RAD2ANGLE(x) ((x) / TAU * 360)

#define YEAR (((__DATE__[9]-'0')) * 10 + (__DATE__[10]-'0'))
#define MONTH (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n' ? 1 : \
            __DATE__[0] == 'F' ? 2 : \
            __DATE__[0] == 'M' && __DATE__[2] == 'r' ? 3 : \
            __DATE__[0] == 'A' && __DATE__[1] == 'p' ? 4 : \
            __DATE__[0] == 'M' ?  5 : \
            __DATE__[0] == 'J' && __DATE__[1] == 'u' ? 6 : \
            __DATE__[0] == 'J' ? 7 : \
            __DATE__[0] == 'A' ? 8 : \
            __DATE__[0] == 'S' ? 9 : \
            __DATE__[0] == 'O' ? 10 : \
            11)

#define DAY ((__DATE__[4] == ' ' ? 0 : __DATE__[4]-'0') * 10 + (__DATE__[5]-'0'))
#define HOUR ((__TIME__[0]-'0') * 10 + __TIME__[1]-'0')
#define MINUTE ((__TIME__[3]-'0') * 10 + __TIME__[4]-'0')
