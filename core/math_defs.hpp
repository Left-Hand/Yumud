#pragma once

#include "core/sys_defs.hpp"
#include <type_traits>
#include "bits/move.h"
#include <bit>

#if defined(YMD_USE_MY_MEMORY)
//TODO 提高memcmp的性能
__fast_inline constexpr int __ymd_memcmp(const void * str1, const void * str2, uint32_t len){
    for (uint32_t i = 0; i < len; i++) {
        if ((static_cast<const uint8_t*>(str1))[i] < (static_cast<const uint8_t*>(str2))[i]) {
            return -1;
        } else if ((static_cast<const uint8_t*>(str1))[i] > (static_cast<const uint8_t*>(str2))[i]) {
            return 1;
        }
    }
    return 0;
}

#define memcmp(str1, str2, len) __ymd_memcmp(str1, str2, len)

//TODO 提高memset的性能
__fast_inline constexpr void * __ymd_memset(void * dest, const char c, const uint32_t len){
	if(std::is_constant_evaluated()){
		return __builtin_memset(dest, c, len);
	}

    for(uint32_t i = 0; i < len; i++){
        reinterpret_cast<char *>(dest)[i] = c;
    }

    return dest;
}

#define memset(source, c, len) __ymd_memset(source, c, len)


//TODO 提高memcpy的性能
__fast_inline constexpr void * __ymd_memcpy(void * dest, const void * src, const uint32_t len){
	if(std::is_constant_evaluated()){
		return __builtin_memcpy(dest, src, len);
	}

    for (uint32_t i = 0; i < len; ++i) {
        (static_cast<uint8_t*>(dest))[i] = (static_cast<const uint8_t*>(src))[i];
    }
    return dest;
}

#define memcpy(dest, source, len) __ymd_memcpy(dest, source, len)

//TODO 提高strlen的性能
__fast_inline constexpr size_t __ymd_strlen(const char * str){
	if(std::is_constant_evaluated()){
		return __builtin_strlen(str);
	}
	
	size_t len = 0;
	while (str[len] != '\0') {
		++len;
	}
	return len;
}

#define strlen(str) __ymd_strlen(str)


//TODO 提高strcpy的性能
__fast_inline constexpr char * __ymd_strcpy(char * dest, const char * src){
    for (; *src != '\0'; ++src, ++dest) {
        *dest = *src;
    }
    return dest;
}

#define strcpy(d, s) __ymd_strcpy(d, s)

#endif

#ifndef YMD_USE_MY_MEMORY
#include "string.h"
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



template<typename First>
constexpr First __max_helper(const First & value) {
    return value;
}

template<typename First, typename Second, typename... Rest>
constexpr First __max_helper(const First & first,const Second & second,const Rest & ... rest){
    First max_value = first > First(second) ? first : First(second);
    return __max_helper(max_value, rest...);
}

template<typename ... Ts >
static constexpr auto MAX(Ts && ... args){return __max_helper(std::forward<Ts>(args)...);}





template<typename First>
constexpr First __min_helper(const First & value) {
    return value;
}

template<typename First, typename Second, typename... Rest>
constexpr First __min_helper(const First & first,const Second & second,const Rest &... rest) {
    First min_value = first < First(second) ? first : First(second);
    return __min_helper(min_value, rest...);
}

template<typename ... Ts >
static constexpr auto MIN(Ts && ... args){return __min_helper(std::forward<Ts>(args)...);}


template <typename T>
requires std::is_arithmetic_v<T>
constexpr __fast_inline T ABS(const T a){
    return (a < 0) ? -a : a;
}



template <typename T>
requires std::is_arithmetic_v<T>
static constexpr __fast_inline T SIGN(const T a){
    return (a < 0) ? T(-1) : T(1);
}




static constexpr bool IN_RANGE(auto x,auto a,auto b) {return((a < b) ? (x >= a && x < b) : (x < a && x >= b));}


void SWAP(auto && m_x, auto && m_y){
    std::swap((m_x), (m_y));
}



template <typename T, typename U>
requires std::is_arithmetic_v<U>
constexpr __fast_inline T LERP(const T & a, const T & b, const U & x){
    return T((U(U(1) - x) * a) + (x * b));
}




template <typename T, typename U, typename V>
constexpr __fast_inline T INVLERP(const U & _a, const V & _b, const T& t){
    T a = static_cast<T>(_a);
    T b = static_cast<T>(_b);
    return (t - a) / (b - a);
}

template<typename T>
constexpr __fast_inline T __clamp_tmpl(const T x, const auto mi, const auto ma) {
    if(unlikely(x > static_cast<T>(ma))) return static_cast<T>(ma);
    if(unlikely(x < static_cast<T>(mi))) return static_cast<T>(mi);
    return x;
}


static constexpr auto CLAMP(auto x,auto mi,auto ma){return __clamp_tmpl(x, mi, ma);}
static constexpr auto CLAMP2(auto x, auto ma){ return __clamp_tmpl(x, -ma, ma);}



template <typename T>
requires std::is_arithmetic_v<T>
constexpr __fast_inline T STEP_TO(const T x,const T y, const T s){
    T err = y-x;
    if(err > s){
        return x + s;
    }else if(err < -s){
        return x - s;
    }else{
        return y;
    }
}



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


template<typename T>
constexpr __fast_inline T LSHIFT(const T x, const int s){
    if (s >= 0){
        return x << s;
    }else{
        return x >> -s;
    }
}


template<typename T>
constexpr __fast_inline T RSHIFT(const T x, const int s){
    if (s >= 0){
        return x >> s;
    }else{
        return x << -s;
    }
}

#define NEXT_POWER_OF_2(x) ((x == 0) ? 1 : (1 << (32 - __builtin_clz(x - 1))))
#define PREV_POWER_OF_2(x) (1 << (31 - __builtin_clz(x)))

#ifndef CTZ
#ifdef __cplusplus
#define CTZ(x) __ymd_ctz_impl(x)
__fast_inline constexpr uint32_t __ymd_ctz_impl(uint32_t x) {
    //https://github.com/microsoft/compiler-rt/blob/master/lib/builtins/ctzsi2.c

    int32_t t = ((x & 0x0000FFFF) == 0) << 4;  /* if (x has no small bits) t = 16 else 0 */
    x >>= t;           /* x = [0 - 0xFFFF] + higher garbage bits */
    uint32_t r = t;       /* r = [0, 16]  */
    /* return r + ctz(x) */
    t = ((x & 0x00FF) == 0) << 3;
    x >>= t;           /* x = [0 - 0xFF] + higher garbage bits */
    r += t;            /* r = [0, 8, 16, 24] */
    /* return r + ctz(x) */
    t = ((x & 0x0F) == 0) << 2;
    x >>= t;           /* x = [0 - 0xF] + higher garbage bits */
    r += t;            /* r = [0, 4, 8, 12, 16, 20, 24, 28] */
    /* return r + ctz(x) */
    t = ((x & 0x3) == 0) << 1;
    x >>= t;
    x &= 3;            /* x = [0 - 3] */
    r += t;            /* r = [0 - 30] and is even */

    return r + ((2 - (x >> 1)) & -((x & 1) == 0));
}

#else
#define CTZ(x) __builtin_ctz((size_t)(x))
#endif
#endif

#ifndef CLZ
#ifdef __cplusplus
#define CLZ(x) __ymd_clz_impl(x)

__fast_inline constexpr uint32_t  __ymd_clz_impl(uint32_t x){
    // https://github.com/m-labs/compiler-rt-lm32/blob/master/lib/clzsi2.c

    int32_t t = ((x & 0xFFFF0000) == 0) << 4;  /* if (x is small) t = 16 else 0 */
    x >>= 16 - t;      /* x = [0 - 0xFFFF] */
    uint32_t r = t;       /* r = [0, 16] */
    /* return r + clz(x) */
    t = ((x & 0xFF00) == 0) << 3;
    x >>= 8 - t;       /* x = [0 - 0xFF] */
    r += t;            /* r = [0, 8, 16, 24] */
    /* return r + clz(x) */
    t = ((x & 0xF0) == 0) << 2;
    x >>= 4 - t;       /* x = [0 - 0xF] */
    r += t;            /* r = [0, 4, 8, 12, 16, 20, 24, 28] */
    /* return r + clz(x) */
    t = ((x & 0xC) == 0) << 1;
    x >>= 2 - t;       /* x = [0 - 3] */
    r += t;            /* r = [0 - 30] and is even */

    return r + ((2 - x) & -((x & 2) == 0));
}

#else
#define CLZ(x) __builtin_clz(((size_t)(x)) << (PLAT_WIDTH - BITS(x)))
#endif
#endif

static constexpr auto BITS(auto x) {return (sizeof(x) * 8);}
static consteval auto PLAT_WIDTH() {return BITS(std::size_t());}

template<typename T>
static constexpr T DEG2RAD(auto x) {return ((x) * static_cast<T>(TAU / 360));}

template<typename T>
static constexpr T RAD2DEG(auto x) {return ((x) * static_cast<T>(360 / TAU));}

static consteval auto YEAR() {return (((__DATE__[9]-'0')) * 10 + (__DATE__[10]-'0'));}
static consteval auto MONTH() {
    return (
        __DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n' ? 1 :
        __DATE__[0] == 'F' ? 2 :
        __DATE__[0] == 'M' && __DATE__[2] == 'r' ? 3 :
        __DATE__[0] == 'A' && __DATE__[1] == 'p' ? 4 :
        __DATE__[0] == 'M' ?  5 :
        __DATE__[0] == 'J' && __DATE__[1] == 'u' ? 6 :
        __DATE__[0] == 'J' ? 7 :
        __DATE__[0] == 'A' ? 8 :
        __DATE__[0] == 'S' ? 9 :
        __DATE__[0] == 'O' ? 10 : 11
    );
}

static consteval auto DAY() {
    return ((__DATE__[4] == ' ' ? 0 : __DATE__[4]-'0') * 10 + (__DATE__[5]-'0'));
}

static consteval auto HOUR() {
    return ((__TIME__[0]-'0') * 10 + __TIME__[1]-'0');
}

static consteval auto MINUTE() {
    return((__TIME__[3]-'0') * 10 + __TIME__[4]-'0');
}
