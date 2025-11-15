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
        return std::sin(T(val * T(TAU)));
    }
    
    template<floating T>
    __fast_inline constexpr T cospu(const T val){
        return cos(val * (1 / TAU));
    }
    
    template<floating T>
    __fast_inline constexpr T inv_sqrt(const T val){
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

    
    template<arithmetic T>
    __fast_inline constexpr T square(const T x) {
        return x * x;
    }

    template<arithmetic T>
    __fast_inline constexpr T distance(const T a, const T b){
        return ABS(a-b);
    }
    
    template<arithmetic T>
    __fast_inline constexpr T normal(const T a, const T b){
        return SIGN(b - a);
    }


    template<floating T>
    std::array<T, 2> sincospu(const T turns){
        const auto theta = turns * static_cast<T>(TAU);
        return {std::sin(theta), std::cos(theta)};
    }

    template<floating T>
    std::array<T, 2> sincos(const T theta){
        return {std::sin(theta), std::cos(theta)};
    }

    template<floating T>
    __fast_inline constexpr T atan2pu(const T a, const T b) {
        constexpr auto INV_TAU = static_cast<T>(1 / TAU);
        return std::atan2(a, b) * INV_TAU;
    }
}
