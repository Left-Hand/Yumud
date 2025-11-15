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


namespace ymd{


    template<size_t Q>
    __fast_inline constexpr fixed_t<31, int32_t> sinf(const fixed_t<Q, int32_t> x){
        return fixed_t<31, int32_t>(iqmath::details::__IQNgetCosSinTemplate<Q>(x.as_bits(), 
            iqmath::details::__IQ31getSinDispatcher));
    }
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<31, int32_t> cosf(const fixed_t<Q, int32_t> x){
        return fixed_t<31, int32_t>(iqmath::details::__IQNgetCosSinTemplate<Q>(x.as_bits(), 
            iqmath::details::__IQ31getCosDispatcher));
    }
    
    template<size_t Q>
    __fast_inline constexpr std::array<fixed_t<31, int32_t>, 2> sincos(const fixed_t<Q, int32_t> x){
        auto res = (iqmath::details::__IQNgetCosSinTemplate<Q>(x.as_bits(), 
            iqmath::details::__IQ31getSinCosDispatcher));
        return {res.sin, res.cos};
    }

    template<size_t Q, typename D>
    requires (sizeof(D) == 4)
    __fast_inline constexpr fixed_t<31, int32_t> sinpu(const fixed_t<Q, D> x){
        return (iqmath::details::__IQNgetCosSinPUTemplate<Q>(x.as_bits(), 
            iqmath::details::__IQ31getSinDispatcher));
    }
    
    template<size_t Q, typename D>
    requires (sizeof(D) == 4)
    __fast_inline constexpr fixed_t<31, int32_t> cospu(const fixed_t<Q, D> x){
        return (iqmath::details::__IQNgetCosSinPUTemplate<Q>(x.as_bits(), 
            iqmath::details::__IQ31getCosDispatcher));
    }
    
    template<size_t Q, typename D>
    requires (sizeof(D) == 4)
    __fast_inline constexpr std::array<fixed_t<31, int32_t>, 2> sincospu(const fixed_t<Q, D> x){
        auto res = (iqmath::details::__IQNgetCosSinPUTemplate<Q>(x.as_bits(), 
            iqmath::details::__IQ31getSinCosDispatcher));
        return {res.sin, res.cos};
    }
    
    template<size_t Q, typename D>
    __fast_inline constexpr fixed_t<31, int32_t> sin(const fixed_t<Q, D> x){return sinf<Q>(x);}
    
    template<size_t Q, typename D>
    __fast_inline constexpr fixed_t<31, int32_t> cos(const fixed_t<Q, D> x){return cosf<Q>(x);}
    
    template<size_t Q, typename D>
    __fast_inline constexpr fixed_t<31, int32_t> tanf(const fixed_t<Q, D> x) {return sinf<Q>(x) / cosf<Q>(x);}
    
    template<size_t Q, typename D>
    __fast_inline constexpr fixed_t<31, int32_t> tan(const fixed_t<Q, D> x) {return tanf<Q>(x);}
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<29, int32_t> asinf(const fixed_t<Q, int32_t> x) {
        return fixed_t<29, int32_t>(iqmath::details::_IQNasin(x));}
    
    template<size_t Q>
    requires (Q < 30)
    __fast_inline constexpr fixed_t<29, int32_t> acosf(const fixed_t<Q, int32_t> x) {
        return fixed_t<29, int32_t>(PI/2) - fixed_t<29, int32_t>(iqmath::details::_IQNasin(x));
    }
    
    template<size_t Q>
    requires (Q < 30)
    __fast_inline constexpr fixed_t<29, int32_t> asin(const fixed_t<Q, int32_t> x){return asinf(x);}
    
    template<size_t Q>
    requires (Q < 30)
    __fast_inline constexpr fixed_t<29, int32_t> acos(const fixed_t<Q, int32_t> x){return acosf(x);}
    
    template<size_t Q>
    requires (Q < 30)
    __fast_inline constexpr fixed_t<Q, int32_t> atanf(const fixed_t<Q, int32_t> x) {
        return fixed_t<Q, int32_t>(iqmath::details::_IQNatan2(x, fixed_t<Q, int32_t>(1)));
    }
    
    template<size_t Q>
    requires (Q < 30)
    __fast_inline constexpr fixed_t<Q, int32_t> atan(const fixed_t<Q, int32_t> x) {
        return atanf(x);
    }

    template<size_t Q>
    requires (Q < 30)
    __fast_inline constexpr fixed_t<Q, int32_t> atan2f(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b) {
        return fixed_t<Q, int32_t>(iqmath::details::_IQNatan2<Q>(a,b));
    }
    
    template<size_t Q>
    requires (Q < 30)
    __fast_inline constexpr fixed_t<Q, int32_t> atan2(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b) {
        return atan2f(a, b);
    }
    
    template<size_t Q>
    requires (Q < 30)
    __fast_inline constexpr fixed_t<Q, int32_t> atan2pu(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b) {
        return iqmath::details::_IQNatan2PU<Q>(a,b);
    }
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> sqrtf(const fixed_t<Q, int32_t> x){
            return fixed_t<Q, int32_t>(iqmath::details::_IQNsqrt(x));
    }
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> sqrt(const fixed_t<Q, int32_t> x){
        return sqrtf(x);
    }
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> log10(const fixed_t<Q, int32_t> x) {
        #ifdef IQ_CH32_LOG
        return fixed_t(_iq<Q>(_IQlog10(x)));
        #else
        return fixed_t<Q, int32_t>(iqmath::details::_IQNlog(x)) / 
            fixed_t(iqmath::details::_IQNlog<Q>(fixed_t<Q, int32_t>::from(10)));
        #endif
    }
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> log(const fixed_t<Q, int32_t> x) {
        #ifdef IQ_CH32_LOG
        return fixed_t(_iq<Q>(_IQdiv(_IQlog10(x), _IQlog10(_iq<Q>(M_E)))));
        #else
            return fixed_t<Q, int32_t>(iqmath::details::_IQNlog(x));
        #endif
    }
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> exp(const fixed_t<Q, int32_t> x) {
        return fixed_t<Q, int32_t>(iqmath::details::_IQNexp<Q>(x));
    }
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> pow(const fixed_t<Q, int32_t> base, const fixed_t<Q, int32_t> exponent) {
        return exp(exponent * log(base));
    }
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> pow(const fixed_t<Q, int32_t> base, const integral auto times) {
        //TODO 判断使用循环还是pow运算 选取最优时间
        return exp(times * log(base));
    }
    
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> inv_sqrt(const fixed_t<Q, int32_t> x){
        return fixed_t<Q, int32_t>(iqmath::details::_IQNisqrt<Q>(x));
    }
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> mag(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b){
        return fixed_t<Q, int32_t>(iqmath::details::_IQNmag<Q>(a, b));
    }
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> inv_mag(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b){
        return fixed_t<Q, int32_t>(iqmath::details::_IQNimag<Q>(a, a));
    }
    
    template<size_t Q>
    static constexpr fixed_t<Q, int32_t> tpzpu(const fixed_t<Q, int32_t> x){
        return abs(4 * frac(x - fixed_t<Q, int32_t>(0.25)) - 2) - 1;
    }
}


namespace std{
    using ymd::fixed_t;

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> sinf(const fixed_t<Q, int32_t> x){return ymd::sinf(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> cosf(const fixed_t<Q, int32_t> x){return ymd::cosf(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> sin(const fixed_t<Q, int32_t> x){return ymd::sin(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> cos(const fixed_t<Q, int32_t> x){return ymd::cos<Q>(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> tanf(const fixed_t<Q, int32_t> x){return ymd::tanf(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> tan(const fixed_t<Q, int32_t> x){return ymd::tan(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> asinf(const fixed_t<Q, int32_t> x){return ymd::asin(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> asin(const fixed_t<Q, int32_t> x){return ymd::asin(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> acosf(const fixed_t<Q, int32_t> x){return ymd::acos(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> acos(const fixed_t<Q, int32_t> x){return ymd::acos(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> atan(const fixed_t<Q, int32_t> x){return ymd::atan(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> atan2f(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b){return ymd::atan2f(a,b);}
    
    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> atan2(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b){return ymd::atan2(a,b);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> sqrt(const fixed_t<Q, int32_t> x){return ymd::sqrt(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> abs(const fixed_t<Q, int32_t> x){return ymd::abs(x);}


    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> fmod(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b){return ymd::fmod(a, b);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> mean(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b){return ymd::mean(a, b);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> floor(const fixed_t<Q, int32_t> x){return ymd::floor(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> ceil(const fixed_t<Q, int32_t> x){return ymd::ceil(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> log10(const fixed_t<Q, int32_t> x){return ymd::log10(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> log(const fixed_t<Q, int32_t> x){return ymd::log(x);}

    template<size_t Q>
    __fast_inline constexpr fixed_t<Q, int32_t> pow(const fixed_t<Q, int32_t> a, const fixed_t<Q, int32_t> b){return ymd::pow(a, b);}

}

