#pragma once



// #if (!defined(USE_IQ)) &&  (!defined(USE_STD_MATH))
// #define USE_STD_MATH
// #endif

// #ifdef USE_IQ
#include "iq/iq_t.hpp"
// #endif

// #if defined(USE_STDMATH)

// #else
// #include "dsp/floatlib/floatlib.hpp"
// #endif

// #ifdef USE_IQ
using real_t = ymd::iq_t<IQ_DEFAULT_Q>;
// #elif defined(USE_DOUBLE)
// using real_t = double;
// #else
// using real_t = float;
// #endif

namespace ymd{

static constexpr real_t pi_4 = real_t(PI/4);
static constexpr real_t pi_2 = real_t(PI/2);
static constexpr real_t pi = real_t(PI);
static constexpr real_t tau = real_t(TAU);


consteval real_t operator"" _r(long double x){
    return real_t(x);
}

consteval real_t operator"" _r(unsigned long long x){
    return real_t(x);
}



__fast_inline constexpr int mean(const int a, const int b){
    return ((a+b) >> 1);
}

template<floating T>
__fast_inline constexpr T mean(const T & a, const T & b){
    return (a+b) / 2.0f;
}

template<typename T>
__fast_inline constexpr T frac(const T fv){
    return (fv - T(int(fv)));
}

template<floating T>
__fast_inline constexpr T round(const T x)
{
    return int(x+0.5f);
}

template<size_t Q = IQ_DEFAULT_Q, size_t P>
__fast_inline constexpr int roundi(const iq_t<P> iq){
    return int(iq + iq_t<Q>(0.5));
}
template<floating T>
__fast_inline constexpr bool is_equal_approx(const T a, const T b) {
    // Check for exact equality first, required to handle "infinity" values.
    if (a == b) {
        return true;
    }
    // Then check for approximate equality.
    auto tolerance = CMP_EPSILON * ABS(a);
    if (tolerance < CMP_EPSILON) {
        tolerance = CMP_EPSILON;
    }
    return ABS(a - b) < tolerance;
}


template<floating T>
__fast_inline constexpr bool is_equal_approx_ratio(const T a, const T b, const T epsilon, const T min_epsilon){
    auto diff = ABS(a - b);
    if (diff == 0.0 || diff < min_epsilon) {
        return true;
    }
    auto avg_size = (ABS(a) + ABS(b)) / 2.0;
    diff /= avg_size;
    return diff < epsilon;
}

template<size_t Q>
bool is_equal_approx(const iq_t<Q> a, const iq_t<Q> b) {
    // Check for exact equality first, required to handle "infinity" values.
    if (a == b) {
        return true;
    }
    // Then check for approximate equality.
    iq_t<Q> tolerance = iq_t<Q>(CMP_EPSILON) * ABS(a);
    if (tolerance < iq_t<Q>(CMP_EPSILON)) {
        tolerance = iq_t<Q>(CMP_EPSILON);
    }
    return ABS(a - b) < tolerance;
}

template<size_t Q>
bool is_equal_approx_ratio(const iq_t<Q> a, const iq_t<Q> b, iq_t<Q> epsilon, iq_t<Q> min_epsilon){

    iq_t<Q> diff = std::abs(a - b);
    if (diff == 0 || diff < min_epsilon) {
        return true;
    }
    iq_t<Q> avg_size = (std::abs(a) + std::abs(b)) >> 1;
    diff /= avg_size;
    return diff < epsilon;
}



template<floating T>
__fast_inline constexpr T sign(const T fv){
    if(fv > 0.0f) return 1.0f;
    else if(fv < 0.0f) return -1.0f;
    return 0.0f;
}


template<arithmetic T>
__fast_inline constexpr T fposmod(T p_x, T p_y) {
    T value = fmod(p_x, p_y);
    if (value < 0) {
        value += p_y;
    }
    return value;
}

template<integral T>
__fast_inline constexpr T sign(const T val){return val == 0 ? 0 : (val < 0 ? -1 : 1);}



__fast_inline constexpr real_t u16_to_uni(const uint16_t data){
    if constexpr(is_fixed_point_v<real_t>){
        constexpr size_t Q = real_t::q_num;
        if constexpr(Q > 16)
            return real_t(_iq<Q>::from_i32(data << (Q - 16)));
        else if constexpr (Q < 16)
            return real_t(_iq<Q>::from_i32(data >> (16 - Q)));
        else
            return real_t(_iq<Q>::from_i32(data));
    }else if constexpr(std::is_floating_point_v<real_t>){
        return real_t(data) / 65536;
    }
}

template<size_t Q>
__fast_inline constexpr iq_t<Q> u32_to_uni(const uint32_t data){
    iq_t<Q> qv;
#if Q > 16
    qv.value = data << (Q - 16);
#elif(Q < 16)
    qv.value = data >> (16 - Q);
#else
    qv.value = _iq(data);
#endif
    return qv;
}

__fast_inline constexpr real_t s16_to_uni(const int16_t data){
    if constexpr(is_fixed_point_v<real_t>){
        return iq_t<16>(data) >> 16;
    }
    return real_t(0);
}

template<size_t Q>
__fast_inline constexpr uint16_t uni_to_u16(const iq_t<Q> qv){
    uint16_t data;
    if constexpr (Q >= 16) data = qv.as_i32() >> (Q - 16);
    else data = qv.as_i32() << (16 - Q);
    if(data == 0 && (qv.as_i32() != 0)) data = 0xffff;
    return data;
}

template<size_t Q>
__fast_inline constexpr int16_t uni_to_s16(const iq_t<Q> qv){
    int16_t data;
#if Q >= 16
    data = qv.value >> (Q - 16);
#else
    data = qv.value << (16 - Q);
#endif
    return data;
}

__fast_inline real_t uni(const uint16_t data){return u16_to_uni(data);}

__fast_inline real_t uni(const int16_t data){return s16_to_uni(data);}

__fast_inline constexpr int warp_mod(const int x, const int y){
    int ret = x % y;
    if(ret < 0) ret += y;
    return ret;
}

}