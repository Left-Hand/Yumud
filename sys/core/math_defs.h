#ifndef __MATH_DEFS_H__

#define __MATH_DEFS_H__

#include "../sys/core/sys_defs.h"

#ifdef __cplusplus
#include <type_traits>
#endif

#define CMP_EPSILON 0.00001
#define CMP_EPSILON2 (CMP_EPSILON * CMP_EPSILON)

#define CMP_NORMALIZE_TOLERANCE 0.000001
#define CMP_POINT_IN_PLANE_EPSILON 0.00001

#ifndef LN2
#define LN2 0.6931471805599453094172321215
#endif

#ifndef TAU
#define TAU 6.2831853071795864769252867666
#endif

#ifndef PI
#define PI 3.1415926535897932384626433833
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

#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif

#ifndef SQRT3
#define SQRT3 1.73205080757f
#endif

#ifndef SQRT2
#define SQRT2 1.41421356237f
#endif

#ifndef LOGE
#define LOGE 0.434294481903
#endif

#ifndef LOG2
#define LOG2 0.301029995664
#endif


#ifndef MAX
#ifdef __cplusplus
    #define MAX(a,b) __max_tmpl(a,b)
    template <typename T, typename U>
    requires std::is_arithmetic_v<T> && std::is_arithmetic_v<U>
    constexpr __fast_inline T __max_tmpl(const T a, const U _b){
        T b = static_cast<T>(_b);
        return (a > b) ? a : b;
    }
#else
    #define MAX(x,y) ((x > y) ? x : y)
#endif
#endif

#ifndef MIN
#ifdef __cplusplus
    #define MIN(a,b) __min_tmpl(a,b)
    template <typename T, typename U>
    requires std::is_arithmetic_v<T> && std::is_arithmetic_v<U>
    constexpr __fast_inline T __min_tmpl(const T a, const U _b){
        T b = static_cast<T>(_b);
        return (a < b) ? a : b;
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

#ifndef IS_ALIGNED
#define IS_ALIGNED(x, a) (((x) & ((typeof(x))(a) - 1)) == 0)
#endif

#ifndef TYPE_CHECK
#define TYPE_CHECK(a,b) (typeof(a) == typeof(b))
#endif

#ifndef IN_RANGE
#define IN_RANGE(x,a,b) ((a < b) ? (x >= a && x < b) : (x < a && x >= b))
#endif

#ifndef SWAP
#ifdef __cplusplus
    #define SWAP(m_x, m_y) __swap_tmpl((m_x), (m_y))
    template <class T>
    constexpr __fast_inline void __swap_tmpl(T &x, T &y) {
        T aux = x;
        x = y;
        y = aux;
    }
#else
    #define SWAP(x,y)   do{typeof(x) t;t=x;x=y;y=t;}while(0)
#endif
#endif

#ifndef LERP
#ifdef __cplusplus
    #define LERP(x,a,b) __lerp_tmpl(x,a,b)
    template <typename T, typename U, typename V>
    requires std::is_arithmetic_v<V>
    constexpr __fast_inline T __lerp_tmpl(const T & x, const U & a, const V & b){
        return a * (T(1) - x) + b * x;
    }
#else
#define LERP(x,a,b) (a * (1 - x) + b * x)
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
#define CLAMP(x, mi, ma) MIN(MAX(x, mi), ma)
#endif

#ifndef STEP_TO
#ifdef __cplusplus
    #define STEP_TO(x, y, s) __step_tmpl(x, y, s)
    template <typename T>
    requires std::is_arithmetic_v<T>
    constexpr __fast_inline T __step_tmpl(const T x,const T y, const T s){
        return CLAMP(y, x - s, x + s);
    }
#else
#define STEP_TO(x, y, s) CLAMP(y, x - s, x + s)
#endif
#endif


#ifndef SIGN_AS
#define SIGN_AS(y,x) ((x > 0) ? y : -y)
#endif

#ifndef LSHIFT
#define LSHIFT(x,s) (s >= 0 ? x << s : x >> -s)
#endif

#ifndef RSHIFT
#define RSHIFT(x,s) LSHIFT(x, -s)
#endif

#define PLAT_WIDTH (sizeof(size_t) * 8)
#define NEXT_POWER_OF_2(x) ((x == 0) ? 1 : (1 << (32 - __builtin_clz(x - 1))))
#define PREV_POWER_OF_2(x) (1 << (31 - __builtin_clz(x)))


#define CTZ(x) __builtin_ctz(x)
#define BIT_WIDTH(x) (sizeof(x) * 8)
#define CLZ(x) __builtin_clz(x << (PLAT_WIDTH - BIT_WIDTH(x)))

#define ANGLE2RAD(x) (x * TAU / 360)
#define RAD2ANGLE(x) (x / TAU * 360)


#endif