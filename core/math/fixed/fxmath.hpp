#pragma once

#include <array>
#include "fixed.hpp"

#include "details/_IQNdiv.hpp"
#include "details/_IQNatan2.hpp"
#include "details/_IQNsqrt.hpp"
#include "details/_IQNexp.hpp"
#include "details/_IQNasin_acos.hpp"
#include "details/_IQNsin_cos.hpp"
#include "details/_IQNlog.hpp"


namespace ymd::math{


template<size_t Q>
static constexpr fixed<Q, int32_t> tpzpu(const fixed<Q, int32_t> x){
    return abs(4 * frac(x - fixed<Q, int32_t>(0.25)) - 2) - 1;
}
}


namespace std{


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto sin(const ymd::math::fixed<Q, D> x){return ymd::math::sin(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto cos(const ymd::math::fixed<Q, D> x){return ymd::math::cos<Q>(x);}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto tan(const ymd::math::fixed<Q, D> x){return ymd::math::tan(x);}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto asin(const ymd::math::fixed<Q, D> x){return ymd::math::asin(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto acos(const ymd::math::fixed<Q, D> x){return ymd::math::acos(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto atan(const ymd::math::fixed<Q, D> x){return ymd::math::atan(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto atan2(const ymd::math::fixed<Q, D> a, const ymd::math::fixed<Q, D> b){return ymd::math::atan2(a,b);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto sqrt(const ymd::math::fixed<Q, D> x){return ymd::math::sqrt(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto abs(const ymd::math::fixed<Q, D> x){return ymd::math::abs(x);}


template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto mod(const ymd::math::fixed<Q, D> a, const ymd::math::fixed<Q, D> b){return ymd::math::mod(a, b);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto mean(const ymd::math::fixed<Q, D> a, const ymd::math::fixed<Q, D> b){return ymd::math::mean(a, b);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto floor(const ymd::math::fixed<Q, D> x){return ymd::math::floor(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto ceil(const ymd::math::fixed<Q, D> x){return ymd::math::ceil(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto log10(const ymd::math::fixed<Q, D> x){return ymd::math::lg(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto log(const ymd::math::fixed<Q, D> x){return ymd::math::ln(x);}

template<size_t Q, typename D>
__attribute__((always_inline)) constexpr 
auto pow(const ymd::math::fixed<Q, D> a, const ymd::math::fixed<Q, D> b){return ymd::math::pow(a, b);}

}

