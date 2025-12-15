#pragma once

#include <array>
#include "fixed_t.hpp"

#include "details/_IQNdiv.hpp"
#include "details/_IQNatan2.hpp"
#include "details/_IQNsqrt.hpp"
#include "details/_IQNexp.hpp"
#include "details/_IQNasin_acos.hpp"
#include "details/_IQNsin_cos.hpp"
#include "details/_IQNlog.hpp"


namespace ymd::math{

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
fixed_t<31, int32_t> sinpu(const fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU<Q>(x.to_bits()).exact_sin();
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
fixed_t<31, int32_t> cospu(const fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU<Q>(x.to_bits()).exact_sin();
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<fixed_t<31, int32_t>, 2> sincospu(const fixed_t<Q, D> x){
    const auto res = iqmath::details::__IQNgetCosSinPU<Q>(x.to_bits()).exact_sincos();
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
fixed_t<31, int32_t> sin(const fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU<Q>(x.to_bits()).exact_sin();
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
fixed_t<31, int32_t> cos(const fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU<Q>(x.to_bits()).exact_sin();
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
std::array<fixed_t<31, int32_t>, 2> sincos(const fixed_t<Q, int32_t> x){
    const auto res = iqmath::details::__IQNgetCosSin<Q>(x.to_bits()).exact_sincos();
    return {res.sin, res.cos};
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<31, int32_t> tan(const fixed_t<Q, int32_t> x) {
    const auto [s, c] = iqmath::details::__IQNgetCosSinPU<Q>(x.to_bits()).exact_sincos();
    return s / c;
}

template<size_t Q>
requires (Q < 30)
__attribute__((always_inline)) constexpr 
fixed_t<29, int32_t> asin(const fixed_t<Q, int32_t> x){
    return fixed_t<29, int32_t>(iqmath::details::_IQNasin(x));
}

template<size_t Q>
requires (Q < 30)
__attribute__((always_inline)) constexpr 
fixed_t<29, int32_t> acos(const fixed_t<Q, int32_t> x){
    return fixed_t<29, int32_t>(M_PI/2) - fixed_t<29, int32_t>(iqmath::details::_IQNasin(x));
}

template<size_t Q>
requires (Q < 30)
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> atan(const fixed_t<Q, int32_t> x) {
    return fixed_t<Q, int32_t>(iqmath::details::_IQNatan2(x, fixed_t<Q, int32_t>(1)));
}

template<size_t Q>
requires (Q < 30)
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> atan2(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b) {
    return iqmath::details::_IQNatan2<Q>(a,b);
}

template<size_t Q>
requires (Q < 30)
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> atan2pu(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b) {
    return iqmath::details::_IQNatan2PU<Q>(a,b);
}


template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> sqrt(const fixed_t<Q, int32_t> x){
    if(x.to_bits() == 0) return 0;
    if(x.to_bits() < 0) __builtin_trap();
    return fixed_t<Q, int32_t>(iqmath::details::_IQNsqrt(
        fixed_t<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(x.to_bits()))
    ));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> ssqrt(const fixed_t<Q, int32_t> x){
    if(x.to_bits() == 0) return 0;
    if(x.to_bits() < 0){
        return -fixed_t<Q, int32_t>(iqmath::details::_IQNsqrt(
            fixed_t<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(-x.to_bits()))
        ));
    }else{
        return fixed_t<Q, int32_t>(iqmath::details::_IQNsqrt(
            fixed_t<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(x.to_bits()))
        ));
    }
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> sqrt(const fixed_t<Q, uint32_t> x){
    if(x.to_bits() == 0) return 0;
    return fixed_t<Q, uint32_t>(iqmath::details::_IQNsqrt(x));
}


template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> sqrt(const fixed_t<Q, int64_t> x){
    if(x.to_bits() == 0) return 0;
    if(x.to_bits() < 0) __builtin_trap();
    return fixed_t<Q, int32_t>(iqmath::details::_IQNsqrt64(
        fixed_t<Q, uint64_t>::from_bits(std::bit_cast<uint64_t>(x.to_bits()))
    ));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> ssqrt(const fixed_t<Q, int64_t> x){
    if(x.to_bits() == 0) return 0;
    if(x.to_bits() < 0){
        return -fixed_t<Q, int32_t>(iqmath::details::_IQNsqrt64(
            fixed_t<Q, uint64_t>::from_bits(std::bit_cast<uint64_t>(-x.to_bits()))
        ));
    }else{
        return fixed_t<Q, int32_t>(iqmath::details::_IQNsqrt64(
            fixed_t<Q, uint64_t>::from_bits(std::bit_cast<uint64_t>(x.to_bits()))
        ));
    }
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> sqrt(const fixed_t<Q, uint64_t> x){
    if(x.to_bits() == 0) return 0;
    return fixed_t<Q, uint32_t>(iqmath::details::_IQNsqrt64(x));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> log10(const fixed_t<Q, int32_t> x) {
    constexpr auto INV_LN10 = 1 / fixed_t(iqmath::details::_IQNlog<Q>(fixed_t<Q, int32_t>(10)));
    return fixed_t<Q, int32_t>(iqmath::details::_IQNlog(x)) * INV_LN10;
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> log(const fixed_t<Q, int32_t> x) {
    return fixed_t<Q, int32_t>(iqmath::details::_IQNlog(x));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> exp(const fixed_t<Q, int32_t> x) {
    return fixed_t<Q, int32_t>(iqmath::details::_IQNexp<Q>(x));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> pow(const fixed_t<Q, int32_t> base, const fixed_t<Q, int32_t> exponent) {
    return exp(exponent * log(base));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> pow(const fixed_t<Q, int32_t> base, const std::integral auto times) {
    //TODO 判断使用循环还是pow运算 选取最优时间
    return exp(times * log(base));
}


template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> inv_sqrt(const fixed_t<Q, int32_t> x){
    return fixed_t<Q, int32_t>(iqmath::details::_IQNisqrt(
        fixed_t<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(x.to_bits()))
    ));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> inv_sqrt(const fixed_t<Q, uint32_t> x){
    return fixed_t<Q, uint32_t>(iqmath::details::_IQNisqrt(x));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> mag(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b){
    return fixed_t<Q, int32_t>(iqmath::details::_IQNmag<Q>(a, b));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> inv_mag(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b){
    return fixed_t<Q, int32_t>(iqmath::details::_IQNimag<Q>(a, b));
}

template<size_t Q>
static constexpr fixed_t<Q, int32_t> tpzpu(const fixed_t<Q, int32_t> x){
    return abs(4 * frac(x - fixed_t<Q, int32_t>(0.25)) - 2) - 1;
}
}


namespace std{
using ymd::fixed_t;


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> sin(const fixed_t<Q, D> x){return ymd::math::sin(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> cos(const fixed_t<Q, D> x){return ymd::math::cos<Q>(x);}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> tan(const fixed_t<Q, D> x){return ymd::math::tan(x);}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> asin(const fixed_t<Q, D> x){return ymd::math::asin(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> acos(const fixed_t<Q, D> x){return ymd::math::acos(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> atan(const fixed_t<Q, D> x){return ymd::math::atan(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> atan2(const fixed_t<Q, D> a, const fixed_t<Q, D> b){return ymd::math::atan2(a,b);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto sqrt(const fixed_t<Q, D> x){return ymd::math::sqrt(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> abs(const fixed_t<Q, D> x){return ymd::math::abs(x);}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> mod(const fixed_t<Q, D> a, const fixed_t<Q, D> b){return ymd::math::mod(a, b);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> mean(const fixed_t<Q, D> a, const fixed_t<Q, D> b){return ymd::math::mean(a, b);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> floor(const fixed_t<Q, D> x){return ymd::math::floor(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> ceil(const fixed_t<Q, D> x){return ymd::math::ceil(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> log10(const fixed_t<Q, D> x){return ymd::math::log10(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> log(const fixed_t<Q, D> x){return ymd::math::log(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<Q, D> pow(const fixed_t<Q, D> a, const fixed_t<Q, D> b){return ymd::math::pow(a, b);}

}

