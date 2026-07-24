#pragma once

#include <array>
#include "fxmath_nontranscend.hpp"
#include "fxmath/atan2.hpp"
#include "fxmath/sqrt.hpp"
#include "fxmath/exp.hpp"
#include "fxmath/asin_acos.hpp"
#include "fxmath/sin_cos.hpp"
#include "fxmath/log.hpp"


namespace ymd::math{


template<size_t Q>
__attribute__((const, always_inline)) static constexpr 
fixed<Q, uint32_t> pow(const fixed<Q, uint32_t> base, const fixed<Q, int32_t> exponent) {
    return exp(exponent * ln(base));
}

template<size_t Q>
__attribute__((const, always_inline)) static constexpr 
fixed<Q, uint32_t> pow(const fixed<Q, uint32_t> base, const int32_t times) {
    //TODO 判断使用循环还是pow运算 选取最优时间
    return exp(times * ln(base));
}

}


namespace std{


template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto sin(const ymd::math::fixed<Q, D> x){return ymd::math::sin(x);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto cos(const ymd::math::fixed<Q, D> x){return ymd::math::cos<Q>(x);}


template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto tan(const ymd::math::fixed<Q, D> x){return ymd::math::tan(x);}


template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto asin(const ymd::math::fixed<Q, D> x){return ymd::math::asin(x);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto acos(const ymd::math::fixed<Q, D> x){return ymd::math::acos(x);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto atan(const ymd::math::fixed<Q, D> x){return ymd::math::atan(x);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto atan2(const ymd::math::fixed<Q, D> a, const ymd::math::fixed<Q, D> b){return ymd::math::atan2(a,b);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto sqrt(const ymd::math::fixed<Q, D> x){return ymd::math::sqrt(x);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto abs(const ymd::math::fixed<Q, D> x){return ymd::math::abs(x);}


template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto mod(const ymd::math::fixed<Q, D> a, const ymd::math::fixed<Q, D> b){return ymd::math::mod(a, b);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto mean(const ymd::math::fixed<Q, D> a, const ymd::math::fixed<Q, D> b){return ymd::math::mean(a, b);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto floor(const ymd::math::fixed<Q, D> x){return ymd::math::floor(x);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto ceil(const ymd::math::fixed<Q, D> x){return ymd::math::ceil(x);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto log10(const ymd::math::fixed<Q, D> x){return ymd::math::lg(x);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto log(const ymd::math::fixed<Q, D> x){return ymd::math::ln(x);}

template<size_t Q, typename D>
__attribute__((const, always_inline)) static constexpr 
auto pow(const ymd::math::fixed<Q, D> a, const ymd::math::fixed<Q, D> b){return ymd::math::pow(a, b);}

}

