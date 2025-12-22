#pragma once

#include <type_traits>

#include "iq/fixed_t.hpp"

using real_t = ymd::math::fixed_t<IQ_DEFAULT_Q, int32_t>;

namespace ymd::literals{
[[nodiscard]] consteval real_t operator"" _r(long double x){
    return real_t(x);
}

[[nodiscard]] consteval real_t operator"" _r(unsigned long long x){
    return real_t(x);
}

}

using namespace ymd::literals;

namespace ymd::math{
    

template<std::integral T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T mean(const T a, const T b){
    return static_cast<T>((a+b) >> 1);
}

template<std::floating_point T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T mean(const T & a, const T & b){
    return (a+b) / 2.0f;
}

template<typename T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T frac(const T fv){
    return (fv - T(int(fv)));
}

template<std::floating_point T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T round(const T x)
{
    return int(x+0.5f);
}




template<typename T, std::integral From>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T floor_cast(From value){
    return static_cast<T>(value);
}

template<typename T, std::integral From>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T ceil_cast(From value){
    return static_cast<T>(value);
}

template<typename T, std::integral From>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T round_cast(From value){
    return static_cast<T>(value);
}


template<std::floating_point T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
bool is_equal_approx(
    const T a, 
    const T b, 
    const T epsilon
) {
    // Check for exact equality first, required to handle "infinity" values.
    if (a == b) {
        return true;
    }
    // Then check for approximate equality.
    auto tolerance = epsilon * ABS(a);
    if (tolerance < epsilon) {
        tolerance = epsilon;
    }
    return ABS(a - b) < tolerance;
}


template<std::floating_point T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
bool is_equal_approx_ratio(const T a, const T b, const T epsilon, const T min_epsilon){
    auto diff = ABS(a - b);
    if (diff == 0.0 || diff < min_epsilon) {
        return true;
    }
    auto avg_size = (ABS(a) + ABS(b)) / 2.0;
    diff /= avg_size;
    return diff < epsilon;
}


template<std::floating_point T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T sign(const T fv){
    if(fv > 0.0f) return 1.0f;
    else if(fv < 0.0f) return -1.0f;
    return 0.0f;
}


template<typename T>
requires (std::is_arithmetic_v<T>)
[[nodiscard]] __attribute__((always_inline)) constexpr 
T fposmod(T p_x, T p_y) {
    T value = fmod(p_x, p_y);
    if (value < 0) {
        value += p_y;
    }
    return value;
}

template<std::integral T>
[[nodiscard]] __attribute__((always_inline)) constexpr 
T sign(const T val){return val == 0 ? 0 : (val < 0 ? -1 : 1);}


#if 0

[[nodiscard]] __attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> u16_to_uni(const uint16_t data){
    if constexpr(is_fixed_point_v<fixed_t<Q, int32_t>>){
        constexpr size_t Q = fixed_t<Q, int32_t>::q_num;
        if constexpr(Q > 16)
            return fixed_t<Q, int32_t>(fixed_t<Q, int32_t>::from_bits(data << (Q - 16)));
        else if constexpr (Q < 16)
            return fixed_t<Q, int32_t>(fixed_t<Q, int32_t>::from_bits(data >> (16 - Q)));
        else
            return fixed_t<Q, int32_t>(fixed_t<Q, int32_t>::from_bits(data));
    }else if constexpr(std::is_floating_point_v<fixed_t<Q, int32_t>>){
        return fixed_t<Q, int32_t>(data) / 65536;
    }
}

template<size_t Q>
[[nodiscard]] __attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> u32_to_uni(const uint32_t data){
    fixed_t<Q, uint32_t> qv;
#if Q > 16
    qv.value = data << (Q - 16);
#elif(Q < 16)
    qv.value = data >> (16 - Q);
#else
    qv.value = _iq(data);
#endif
    return qv;
}

[[nodiscard]] __attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> s16_to_uni(const int16_t data){
    if constexpr(is_fixed_point_v<fixed_t<Q, int32_t>>){
        return fixed_t<16, int32_t>(data) >> 16;
    }
    return fixed_t<Q, int32_t>(0);
}

template<size_t Q>
[[nodiscard]] __attribute__((always_inline)) constexpr 
uint16_t uni_to_u16(const fixed_t<Q, uint32_t> qv){
    uint16_t data;
    if constexpr (Q >= 16) data = qv.to_bits() >> (Q - 16);
    else data = qv.to_bits() << (16 - Q);
    if(data == 0 && (qv.to_bits() != 0)) data = 0xffff;
    return data;
}

template<size_t Q>
[[nodiscard]] __attribute__((always_inline)) constexpr 
int16_t uni_to_s16(const fixed_t<Q, int32_t> qv){
    int16_t data;
#if Q >= 16
    data = qv.value >> (Q - 16);
#else
    data = qv.value << (16 - Q);
#endif
    return data;
}

[[nodiscard]] __attribute__((always_inline)) fixed_t<
Q, int32_t> uni(const uint16_t data){return u16_to_uni(data);}

[[nodiscard]] __attribute__((always_inline)) fixed_t<
Q, int32_t> uni(const int16_t data){return s16_to_uni(data);}

#endif

[[nodiscard]] __attribute__((always_inline)) constexpr 
int warp_mod(const int x, const int y){
    int ret = x % y;
    if(ret < 0) ret += y;
    return ret;
}

}