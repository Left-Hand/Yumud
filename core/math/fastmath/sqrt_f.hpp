#pragma once

#include "core/platform.hpp"
#include <bit>

namespace ymd::ffm{
template<floating T>
__fast_inline constexpr T fast_inv_sqrt(T fv){
    if constexpr(std::is_same_v<T,float>){
        uint32_t u = std::bit_cast<uint32_t>(fv);
        u = 0x5f375a86 - (u>>1); // gives initial guess y0
        return std::bit_cast<T>(u);
    }else if constexpr(std::is_same_v<T,double>){
        uint64_t u = std::bit_cast<uint64_t>(fv);
        u = 0x5fe6eb50c7b537a9 - (u>>1); // gives initial guess y0
        return std::bit_cast<T>(u);
    }else{
        __builtin_unreachable();
    }
}

template<floating T>
__fast_inline constexpr T fast_sqrt(T fv) {
    return fv * fast_inv_sqrt(fv);
}

template<floating T>
__fast_inline constexpr T fast_rcp(T fv){
    T temp = fast_inv_sqrt(fv);
    return temp * temp; 
}


__fast_inline constexpr float inv_sqrt(const float number){return fast_inv_sqrt(number);}
__fast_inline constexpr float sqrt(const float number){return fast_sqrt(number);}
__fast_inline constexpr float rcp(const float number){return fast_rcp(number);}

__fast_inline constexpr double inv_sqrt(const double number){return fast_inv_sqrt(number);}
__fast_inline constexpr double sqrt(const double number){return fast_sqrt(number);}
__fast_inline constexpr double rcp(const double number){return fast_rcp(number);}

}
