#pragma once

#include "real.hpp"

#include "core/math_defs.hpp"

#ifdef USE_IQ
#include "iq/iqmath.hpp"
#endif

#include <cmath>
#include "dsp/constexprmath/ConstexprMath.hpp"

namespace ymd{

    
    template<floating T>
    __fast_inline constexpr T powfi(const T base, const int exponent) {
        if(0 == exponent) {
            return T(1);
        }else if(1 == exponent){
            return base;
        }else{
            T ret;
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
    __fast_inline constexpr T powi(const T base, const int exponent) {
        return powi(base, exponent);
    }
    
    template<floating T>
    __fast_inline constexpr T sinpu(const T val){
        return sin(val * (1 / TAU));
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
        return sqrt(a * a + b * b);
    }

    template<floating T>
    __fast_inline constexpr std::array<T, 2> sincos(const T x){
        return {std::sin(x), std::cos(x)};
    }
    
    template<arithmetic T>
    __fast_inline constexpr T square(const T x) {
        return x * x;
    }
    
    template<arithmetic T>
    __fast_inline constexpr T round(const T x) {
        const int i = int(floor(x));
        return T(i) + T(int(bool(x - i >= 0.5)));
    }
    
    
    __fast_inline constexpr real_t distance(const real_t & a, const real_t & b){
        return ABS(a-b);
    }
    
    __fast_inline constexpr real_t normal(const real_t & a, const real_t & b){
        return SIGN(b - a);
    }
    }
    
    