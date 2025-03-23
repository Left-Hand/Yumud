#pragma once

#include <array>
#include "iq_t.hpp"

#include "universal/_IQNdiv.hpp"
#include "universal/_IQNatan2.hpp"
#include "universal/_IQNtoF.hpp"
#include "universal/_IQFtoN.hpp"
#include "universal/_IQNsqrt.hpp"
#include "universal/_IQNexp.hpp"
#include "universal/_IQNasin_acos.hpp"
#include "universal/_IQNsin_cos.hpp"
#include "universal/_IQNlog.hpp"


namespace ymd{


    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> sinf(const iq_t<P> iq_x){
        return iq_t<Q>(__iqdetails::__IQNgetCosSinTemplate<P>(iq_x.value.to_i32(), __iqdetails::__IQ31getSinDispatcher));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> cosf(const iq_t<P> iq_x){
        return iq_t<Q>(__iqdetails::__IQNgetCosSinTemplate<P>(iq_x.value.to_i32(), __iqdetails::__IQ31getCosDispatcher));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr std::array<iq_t<Q>, 2> sincos(const iq_t<P> iq_x){
        auto res = (__iqdetails::__IQNgetCosSinTemplate<Q>(iq_x.value.to_i32(), __iqdetails::__IQ31getSinCosDispatcher));
        return {res.sin, res.cos};
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> sinpu(const iq_t<P> iq_x){
        return iq_t<Q>(__iqdetails::__IQNgetCosSinPUTemplate<Q>(iq_x.value.to_i32(), __iqdetails::__IQ31getSinDispatcher));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> cospu(const iq_t<P> iq_x){
        return iq_t<Q>(__iqdetails::__IQNgetCosSinPUTemplate<Q>(iq_x.value.to_i32(), __iqdetails::__IQ31getCosDispatcher));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr std::array<iq_t<Q>, 2> sincospu(const iq_t<P> iq_x){
        auto res = (__iqdetails::__IQNgetCosSinPUTemplate<Q>(iq_x.value.to_i32(), __iqdetails::__IQ31getSinCosDispatcher));
        return {res.sin, res.cos};
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> sin(const iq_t<P> iq){return sinf<Q>(iq);}
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> cos(const iq_t<P> iq){return cosf<Q>(iq);}
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> tanf(const iq_t<P> iq) {return sinf<Q>(iq) / cosf<Q>(iq);}
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> tan(const iq_t<P> iq) {return tanf<Q>(iq);}
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> asinf(const iq_t<P> iq) {return iq_t<29>(__iqdetails::_IQNasin(iq.value));}
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    requires (Q < 30)
    __fast_inline constexpr iq_t<29> acosf(const iq_t<P> iq) {
        return iq_t<29>(PI/2) - iq_t<29>(__iqdetails::_IQNasin(iq.value));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    requires (Q < 30)
    __fast_inline constexpr iq_t<Q> asin(const iq_t<P> iq){return asinf(iq);}
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    requires (Q < 30)
    __fast_inline constexpr iq_t<Q> acos(const iq_t<P> iq){return acosf(iq);}
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    requires (Q < 30)
    __fast_inline constexpr iq_t<Q> atanf(const iq_t<P> iq) {
        return iq_t<Q>(__iqdetails::_IQNatan2(iq.value, iq_t<P>(1).value));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    requires (Q < 30)
    __fast_inline constexpr iq_t<Q> atan(const iq_t<P> iq) {
        return atanf(iq);
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    requires (Q < 30)
    __fast_inline constexpr iq_t<Q> atan2f(const iq_t<P> a, const iq_t<P> b) {
        return iq_t<Q>(_iq<Q>(__iqdetails::_IQNatan2<Q>(a.value,b.value)));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    requires (Q < 30)
    __fast_inline constexpr iq_t<Q> atan2(const iq_t<P> a, const iq_t<P> b) {
        return atan2f(a, b);
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> sqrtf(const iq_t<P> iq){
            return iq_t<Q>(__iqdetails::_IQNsqrt(iq.value));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> sqrt(const iq_t<P> iq){
        return sqrtf(iq);
    }
    


    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> log10(const iq_t<P> iq) {
        #ifdef IQ_CH32_LOG
        return iq_t(_iq<Q>(_IQlog10(iq.value)));
        #else
        return iq_t<Q>(__iqdetails::_IQNlog(iq.value)) / iq_t(__iqdetails::_IQNlog<Q>(iq_t<Q>::from(10).value));
        #endif
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> log(const iq_t<P> iq) {
        #ifdef IQ_CH32_LOG
        return iq_t(_iq<Q>(_IQdiv(_IQlog10(iq.value), _IQlog10(_iq<Q>(M_E)))));
        #else
            return iq_t<Q>(__iqdetails::_IQNlog(iq.value));
        #endif
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> exp(const iq_t<P> iq) {
        return iq_t<Q>(__iqdetails::_IQNexp<Q>(iq.value));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> pow(const iq_t<P> base, const iq_t<P> exponent) {
        return exp(exponent * log(base));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> pow(const iq_t<P> base, const integral auto times) {
        //TODO 判断使用循环还是pow运算 选取最优时间
        return exp(times * log(base));
    }
    
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> isqrt(const iq_t<P> iq){
        return iq_t<Q>(__iqdetails::_IQNisqrt<P>(iq.value));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> mag(const iq_t<P> a, const iq_t<P> b){
        return iq_t<Q>(__iqdetails::_IQNmag<P>(a.value, b.value));
    }
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> imag(const iq_t<P> a, const iq_t<P> b){
        return iq_t<Q>(__iqdetails::_IQNimag<P>(a.value, a.value));
    }
    
    
}


namespace std{
    using ymd::iq_t;

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> sinf(const iq_t<P> iq){return ymd::sinf(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> cosf(const iq_t<P> iq){return ymd::cosf(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> sin(const iq_t<P> iq){return ymd::sin(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> cos(const iq_t<P> iq){return ymd::cos<Q>(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> tanf(const iq_t<P> iq){return ymd::tanf(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> tan(const iq_t<P> iq){return ymd::tan(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> asinf(const iq_t<P> iq){return ymd::asin(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> asin(const iq_t<P> iq){return ymd::asin(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> acosf(const iq_t<P> iq){return ymd::acos(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> acos(const iq_t<P> iq){return ymd::acos(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> atan(const iq_t<P> iq){return ymd::atan(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> atan2f(const iq_t<P> a, const iq_t<P> b){return ymd::atan2f(a,b);}
    
    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> atan2(const iq_t<P> a, const iq_t<P> b){return ymd::atan2(a,b);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> sqrt(const iq_t<P> iq){return ymd::sqrt(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> abs(const iq_t<P> iq){return ymd::abs(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr bool isnormal(const iq_t<P> iq){return ymd::isnormal(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr bool signbit(const iq_t<P> iq){return ymd::signbit(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> fmod(const iq_t<P> a, const iq_t<P> b){return ymd::fmod(a, b);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> mean(const iq_t<P> a, const iq_t<P> b){return ymd::mean(a, b);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> floor(const iq_t<P> iq){return ymd::floor(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> ceil(const iq_t<P> iq){return ymd::ceil(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> log10(const iq_t<P> iq){return ymd::log10(iq);}

    template<size_t Q = IQ_DEFAULT_Q, size_t P>
    __fast_inline constexpr iq_t<Q> log(const iq_t<P> iq){return ymd::log(iq);}

}

