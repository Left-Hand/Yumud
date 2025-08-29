#pragma once

#include "real.hpp"
#include "core/math_defs.hpp"

#include <cmath>
#include "dsp/constexprmath/ConstexprMath.hpp"

#include "iq/iqmath.hpp"

namespace ymd{

    
    template<arithmetic T>
    __fast_inline constexpr T powfi(const T base, const size_t exponent) {
        if(0 == exponent) {
            return T(1);
        }else if(1 == exponent){
            return base;
        }else{
            T ret = 0;
            if(1 < exponent){
                for(size_t i = 1; i < exponent; ++i){
                    ret *= base;
                }
            }else{
                for(size_t i = 1; i < -exponent; ++i){
                    ret /= base;
                }
            }
            return ret;
        }
    }
    
    template<floating T>
    __fast_inline constexpr T sinpu(const T val){
        return std::sin(float(val * float(TAU)));
    }
    
    template<floating T>
    __fast_inline constexpr T cospu(const T val){
        return cos(val * (1 / TAU));
    }
    
    __fast_inline constexpr float isqrt(const float val){
        if(std::is_constant_evaluated()) return 1 /ConstexprMath::sqrt(val);
        else return 1 / std::sqrt(val);
    }
    
    template<floating T>
    __fast_inline constexpr T imag(const T a, const T b){
        return 1 / mag(a,b);
    }
    
    template<floating T>
    __fast_inline constexpr  T mag(const T a, const T b){
        return std::sqrt(a * a + b * b);
    }

    template<floating T>
    __fast_inline constexpr std::array<T, 2> sincos(const T x){
        return {std::sin(x), std::cos(x)};
    }
    
    template<arithmetic T>
    __fast_inline constexpr T square(const T x) {
        return x * x;
    }
    
    template<size_t N>
    __fast_inline constexpr iq_t<N> square(const iq_t<N> x) {
        return x * x;
    }
    
    __fast_inline constexpr real_t distance(const real_t & a, const real_t & b){
        return ABS(a-b);
    }
    
    __fast_inline constexpr real_t normal(const real_t & a, const real_t & b){
        return SIGN(b - a);
    }

    template<size_t N>
    static __fast_inline constexpr iq_t<N> errmod(const iq_t<N> x, const iq_t<N> s) {
        const auto s_by_2 = s >> 1;
        iq_t<N> value = fmod(x, s);
        if (value > s_by_2) {
            value -= s;
        } else if (value <= -s_by_2) {
            value += s;
        }
        return value;
    }
}
