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
math::fixed_t<31, int32_t> sin(const math::fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU(rad_to_uq32(x).to_bits())
        .exact_sin(iqmath::details::__SincosIntermediate::exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
math::fixed_t<31, int32_t> cos(const math::fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU(rad_to_uq32(x).to_bits())
        .exact_cos(iqmath::details::__SincosIntermediate::exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
math::fixed_t<31, int32_t> sin_approx(const math::fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU(rad_to_uq32(x).to_bits())
        .exact_sin(iqmath::details::__SincosIntermediate::exact_laws::taylor_2o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
math::fixed_t<31, int32_t> cos_approx(const math::fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU(rad_to_uq32(x).to_bits())
        .exact_cos(iqmath::details::__SincosIntermediate::exact_laws::taylor_2o);
}


template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
math::fixed_t<31, int32_t> sinpu(const math::fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU(pu_to_uq32(x).to_bits())
        .exact_sin(iqmath::details::__SincosIntermediate::exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
math::fixed_t<31, int32_t> cospu(const math::fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU(pu_to_uq32(x).to_bits())
        .exact_cos(iqmath::details::__SincosIntermediate::exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
math::fixed_t<31, int32_t> sinpu_approx(const math::fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU(pu_to_uq32(x).to_bits())
        .exact_sin(iqmath::details::__SincosIntermediate::exact_laws::taylor_2o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
math::fixed_t<31, int32_t> cospu_approx(const math::fixed_t<Q, D> x){
    return iqmath::details::__IQNgetCosSinPU(pu_to_uq32(x).to_bits())
        .exact_cos(iqmath::details::__SincosIntermediate::exact_laws::taylor_2o);
}


template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<math::fixed_t<31, int32_t>, 2> sincos(const math::fixed_t<Q, D> x){
    const auto res = iqmath::details::__IQNgetCosSinPU(rad_to_uq32(x).to_bits())
        .exact_sincos(iqmath::details::__SincosIntermediate::exact_laws::taylor_3o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<math::fixed_t<31, int32_t>, 2> sincospu(const math::fixed_t<Q, D> x){
    const auto res = iqmath::details::__IQNgetCosSinPU(pu_to_uq32(x).to_bits())
        .exact_sincos(iqmath::details::__SincosIntermediate::exact_laws::taylor_3o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<math::fixed_t<31, int32_t>, 2> sincospu_approx(const math::fixed_t<Q, D> x){
    const auto res = iqmath::details::__IQNgetCosSinPU(pu_to_uq32(x).to_bits())
        .exact_sincos(iqmath::details::__SincosIntermediate::exact_laws::taylor_2o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<math::fixed_t<31, int32_t>, 2> sincos_approx(const math::fixed_t<Q, D> x){
    const auto res = iqmath::details::__IQNgetCosSinPU(rad_to_uq32(x).to_bits())
        .exact_sincos(iqmath::details::__SincosIntermediate::exact_laws::taylor_2o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<16, int32_t> tan(const fixed_t<Q, D> x) {
    const auto [s, c] = sincos(x);
    return iq16(s) / iq16(c);
}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<16, int32_t> tanpu(const fixed_t<Q, D> x) {
    const auto [s, c] = sincospu(x);
    return iq16(s) / iq16(c);
}


//为了避免计算tan的倒数时调用了两次除法 提供cot函数
template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<16, int32_t> cot(const fixed_t<Q, D> x) {
    const auto [s, c] = sincos(x);
    return iq16(c) / iq16(s);
}



//为了避免计算tan的倒数时调用了两次除法 提供cot函数
template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
fixed_t<16, int32_t> cotpu(const fixed_t<Q, D> x) {
    const auto [s, c] = sincospu(x);
    return iq16(c) / iq16(s);
}


template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<29, int32_t> asin(const fixed_t<Q, int32_t> x){
    return fixed_t<29, int32_t>(iqmath::details::_IQNasin(x));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<29, int32_t> acos(const fixed_t<Q, int32_t> x){
    return fixed_t<29, int32_t>(M_PI/2) - asin(x);
}


template<size_t Q>
constexpr ymd::math::fixed_t<32, uint32_t> atan2pu(
    ymd::math::fixed_t<Q, int32_t> iqn_input_y, 
    ymd::math::fixed_t<Q, int32_t> iqn_input_x)
{
    return ymd::math::fixed_t<32, uint32_t>::from_bits(
        std::bit_cast<int32_t>(iqmath::details::_atan2pu_impl<Q>(iqn_input_y.to_bits(), iqn_input_x.to_bits()).to_bits())
    );
}



template<size_t Q>
constexpr ymd::math::fixed_t<29, int32_t> atan2(
    ymd::math::fixed_t<Q, int32_t> iqn_input_y, 
    ymd::math::fixed_t<Q, int32_t> iqn_input_x)
{
    return ymd::math::uq32_to_rad(atan2pu<Q>(iqn_input_y, iqn_input_x));
}

template<size_t Q>
constexpr ymd::math::fixed_t<32, uint32_t> atanpu(
    ymd::math::fixed_t<Q, int32_t> iqn_input_y
){
    return iqmath::details::_atanpu_impl<Q>(iqn_input_y.to_bits());
}


template<size_t Q>
constexpr ymd::math::fixed_t<29, int32_t> atan(
    ymd::math::fixed_t<Q, int32_t> iqn_input_y
){
    return ymd::math::uq32_to_rad(atanpu<Q>(iqn_input_y));
}



template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> sqrt(const fixed_t<Q, int32_t> x){
    if(x.to_bits() == 0) return 0;
    if(x.to_bits() < 0) __builtin_trap();
    return fixed_t<Q, int32_t>(iqmath::details::_IQNsqrt32(
        fixed_t<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(x.to_bits()))
    ));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> ssqrt(const fixed_t<Q, int32_t> x){
    if(x.to_bits() == 0) return 0;
    if(x.to_bits() < 0){
        return -fixed_t<Q, int32_t>(iqmath::details::_IQNsqrt32(
            fixed_t<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(-x.to_bits()))
        ));
    }else{
        return fixed_t<Q, int32_t>(iqmath::details::_IQNsqrt32(
            fixed_t<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(x.to_bits()))
        ));
    }
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> sqrt(const fixed_t<Q, uint32_t> x){
    if(x.to_bits() == 0) return 0;
    return fixed_t<Q, uint32_t>(iqmath::details::_IQNsqrt32(x));
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
fixed_t<Q, int32_t> log10(const fixed_t<Q, uint32_t> x) {
    constexpr auto INV_LN10 = 1 / fixed_t(iqmath::details::_IQNlog<Q>(fixed_t<Q, int32_t>(10)));
    return fixed_t<Q, int32_t>(iqmath::details::_IQNlog(x)) * INV_LN10;
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> log(const fixed_t<Q, uint32_t> x) {
    return fixed_t<Q, int32_t>(iqmath::details::_IQNlog(x));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> exp(const fixed_t<Q, int32_t> x) {
    return fixed_t<Q, uint32_t>(iqmath::details::_IQNexp<Q>(x));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> pow(const fixed_t<Q, uint32_t> base, const fixed_t<Q, int32_t> exponent) {
    return exp(exponent * log(base));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> pow(const fixed_t<Q, uint32_t> base, const std::integral auto times) {
    //TODO 判断使用循环还是pow运算 选取最优时间
    return exp(times * log(base));
}


template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, int32_t> inv_sqrt(const fixed_t<Q, int32_t> x){
    return fixed_t<Q, int32_t>(iqmath::details::_IQNisqrt32(
        fixed_t<Q, uint32_t>::from_bits(std::bit_cast<uint32_t>(x.to_bits()))
    ));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> inv_sqrt(const fixed_t<Q, uint32_t> x){
    return fixed_t<Q, uint32_t>(iqmath::details::_IQNisqrt32<Q>(x));
}


template<typename D, size_t Q, typename... Args>
__attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> mag(const fixed_t<Q, D> first, Args&&... rest) {
    return fixed_t<Q, uint32_t>(iqmath::details::_IQNmag(first, rest...));
}

template<typename D, size_t Q, typename... Args>
__attribute__((always_inline)) constexpr 
fixed_t<Q, uint32_t> inv_mag(const fixed_t<Q, D> first, Args&&... rest) {
    return fixed_t<Q, uint32_t>(iqmath::details::_IQNimag(first, rest...));
}


template<size_t Q>
static constexpr fixed_t<Q, int32_t> tpzpu(const fixed_t<Q, int32_t> x){
    return abs(4 * frac(x - fixed_t<Q, int32_t>(0.25)) - 2) - 1;
}
}


namespace std{


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> sin(const ymd::math::fixed_t<Q, D> x){return ymd::math::sin(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> cos(const ymd::math::fixed_t<Q, D> x){return ymd::math::cos<Q>(x);}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> tan(const ymd::math::fixed_t<Q, D> x){return ymd::math::tan(x);}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> asin(const ymd::math::fixed_t<Q, D> x){return ymd::math::asin(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> acos(const ymd::math::fixed_t<Q, D> x){return ymd::math::acos(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> atan(const ymd::math::fixed_t<Q, D> x){return ymd::math::atan(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> atan2(const ymd::math::fixed_t<Q, D> a, const ymd::math::fixed_t<Q, D> b){return ymd::math::atan2(a,b);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto sqrt(const ymd::math::fixed_t<Q, D> x){return ymd::math::sqrt(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> abs(const ymd::math::fixed_t<Q, D> x){return ymd::math::abs(x);}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> mod(const ymd::math::fixed_t<Q, D> a, const ymd::math::fixed_t<Q, D> b){return ymd::math::mod(a, b);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> mean(const ymd::math::fixed_t<Q, D> a, const ymd::math::fixed_t<Q, D> b){return ymd::math::mean(a, b);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> floor(const ymd::math::fixed_t<Q, D> x){return ymd::math::floor(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> ceil(const ymd::math::fixed_t<Q, D> x){return ymd::math::ceil(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> log10(const ymd::math::fixed_t<Q, D> x){return ymd::math::log10(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> log(const ymd::math::fixed_t<Q, D> x){return ymd::math::log(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<Q, D> pow(const ymd::math::fixed_t<Q, D> a, const ymd::math::fixed_t<Q, D> b){return ymd::math::pow(a, b);}

}

