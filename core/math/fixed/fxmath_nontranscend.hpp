#pragma once

#include <array>
#include "fixed.hpp"


namespace ymd::math{

    
template<size_t Q1, size_t Q2, 
    typename D1, typename D2, 
    typename D = tmp::extended_mul_underlying_t<D1, D2>
    >
static constexpr auto extended_mul(const fixed<Q1, D1> a, const fixed<Q2, D2> b) 
-> fixed<Q1 + Q2, D>{
    return fixed<Q1 + Q2, D>::from_bits(static_cast<D>(a.to_bits()) * static_cast<D>(b.to_bits()));
}


template<size_t Q1, size_t Q2>
static constexpr fixed<Q1, int32_t> sat(
    const fixed<Q1, int32_t> x, const fixed<Q2, int32_t> k
){
    const auto kx = extended_mul(x, k);
    constexpr auto mask = (std::numeric_limits<uint64_t>::max() << (Q1 + Q2));
    if(kx.to_bits() & mask){
        if(kx < 0) return -1;
        else return 1;
    }else{
        return fixed<Q1, int32_t>::from_bits(kx.to_bits() >> Q2);
    }
}



template<size_t Q, typename D>
static constexpr fixed<Q, D> closer_to_zero(const fixed<Q, D> a, const fixed<Q, D> b){
    if constexpr(std::is_signed_v<D>){
        if(math::abs(a) < math::abs(b)){
            return a;
        }else{
            return b;
        }
    }else{
        if(a < b){
            return a;
        }else{
            return b;
        }
    }
}




template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, D> sign(const fixed<Q, D> x){
    static_assert(fixed<Q, D>(-1) != fixed<Q, D>(1));
    if((x)) [[likely]]return fixed<Q, D>(x > 0 ? 1 : -1);
    else return fixed<Q, D>(0);
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, D> mod(const fixed<Q, D> a, const fixed<Q, D> b){
    return fixed<Q, D>(fixed<Q, D>::from_bits(a.to_bits() % b.to_bits()));
}

template<size_t Q, typename D, typename U = std::make_unsigned_t<D>>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, U> fposmod(const fixed<Q, D> a, const fixed<Q, D> b){
    constexpr size_t SHIFT = size_t(sizeof(D) * 8 - 1);
    const D rem = std::bit_cast<D>(a.to_bits() % b.to_bits());
    const D is_negative = static_cast<D>(rem >> SHIFT);  // 符号位扩展（0 或 -1）
    return fixed<Q, U>::from_bits(static_cast<U>(rem + static_cast<U>(b.to_bits() & is_negative)));
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, D> lerp(const fixed<Q, D> x, const fixed<Q, D> a, const fixed<Q, D> b){
    return a * (1 - x) + b * x;
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, D> mean(const fixed<Q, D> a, const fixed<Q, D> b){
    return fixed<Q, D>(fixed<Q, D>::from_bits((a.to_bits() + b.to_bits()) >> 1));}

template<size_t Q, typename D, typename U = std::make_unsigned_t<D>>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, U> frac(const fixed<Q, D> x){
    constexpr U MASK = static_cast<U>((uint64_t(1u) << Q) - 1);
    return fixed<Q, U>::from_bits(static_cast<U>(static_cast<U>(x.to_bits()) & MASK));
}


template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
D floor_int(const fixed<Q, D> x){
    return static_cast<D>(x.to_bits() >> Q);}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
D ceil_int(const fixed<Q, D> x){
    constexpr D MASK = (1 << Q) - 1;
    return static_cast<D>((x.to_bits() >> Q) + bool(x.to_bits() & MASK));
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
D round_int(const fixed<Q, D> x){
    constexpr D MASK = (1 << (Q - 1));
    return static_cast<D>((x.to_bits() + MASK) >> Q);
}

template<typename T, size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
T floor_cast(const fixed<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(floor_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<typename T, size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
T ceil_cast(const fixed<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(ceil_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<typename T, size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
T round_cast(const fixed<Q, D> x){
    if constexpr(std::is_integral_v<T>){
        return static_cast<T>(round_int(x));
    }else{
        return static_cast<T>(x);
    }
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, D> floor(const fixed<Q, D> x){
    return floor_int(x);
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, D> ceil(const fixed<Q, D> x){
    return ceil_int(x);
}



template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, D> round(const fixed<Q, D> x){
    return round_int(x);
}



template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, D> square(const fixed<Q, D> x) {
    return x * x;
}


template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr 
fixed<Q, D> cubic(const fixed<Q, D> x) {
    return x * x * x;
}


template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
fixed<Q, D> errmod(const fixed<Q, D> x, const fixed<Q, D> s) {
    const auto s_by_2 = s >> 1;
    fixed<Q, D> value = mod(x, s);
    if (value > s_by_2) {
        value -= s;
    } else if (value <= -s_by_2) {
        value += s;
    }
    return value;
}

template<size_t Q_to, size_t Q_from, typename D>
__attribute__((const, always_inline)) static constexpr  
fixed<Q_to, int32_t> fixed_downcast(const fixed<Q_from, D> val){
    return fixed<Q_to, int32_t>::from_bits(static_cast<int32_t>(val.to_bits() >> (Q_from - Q_to)));
}


template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr bool 
is_equal_approx(
    const fixed<Q, D> a, 
    const fixed<Q, D> b,
    const fixed<Q, D> epsilon
) {
    // Check for exact equality first, required to handle "infinity" values.
    if (a - b == D(0)) {
        return true;
    }
    // Then check for approximate equality.
    fixed<Q, D> tolerance = fixed<Q, D>() * (a < 0 ? -a : a);
    if (tolerance < fixed<Q, D>(epsilon)) {
        tolerance = fixed<Q, D>(epsilon);
    }
    return ((a - b < 0) ? b - a : a - b) < tolerance;
}

template<size_t Q, typename D>
[[nodiscard]] __attribute__((const, always_inline)) static constexpr bool 
is_equal_approx_ratio(
    const fixed<Q, D> a, 
    const fixed<Q, D> b, 
    fixed<Q, D> epsilon, 
    fixed<Q, D> min_epsilon
){

    fixed<Q, D> diff = ymd::math::abs(a - b);
    if (diff == 0 || diff < min_epsilon) {
        return true;
    }
    fixed<Q, D> avg_size = (ymd::math::abs(a) + ymd::math::abs(b)) >> 1;
    diff = diff / avg_size;
    return diff < epsilon;
}



template<size_t Q>
__attribute__((const, always_inline)) static constexpr 
fixed<Q, int32_t> tpzpu(const fixed<Q, int32_t> x){
    return abs(4 * frac(x - fixed<Q, int32_t>(0.25)) - 2) - 1;
}



}

