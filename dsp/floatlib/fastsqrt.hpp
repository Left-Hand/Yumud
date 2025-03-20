#pragma once

#include "../core/platform.h"
#include <bit>


// static __fast_inline float int2float(int number) {
//     union {
//         int i;
//         float f;
//     } converter;
//     converter.i = number;
//     return converter.f;
// }

// static __fast_inline int float2int(float number) {
//     union {
//         int i;
//         float f;
//     } converter;
//     converter.f = number;
//     return converter.i;
// }



namespace ffm{
template<floating T>
__fast_inline T fast_invsqrt(T fv){
    if constexpr(std::is_same_v<T,float>){
        uint32_t u = std::bit_cast<uint32_t>(fv);
        u = 0x5f375a86 - (u>>1); // gives initial guess y0
        return std::bit_cast<T>(u);
    }else if constexpr(std::is_same_v<T,double>){
        uint64_t u = std::bit_cast<uint64_t>(fv);
        u = 0x5fe6eb50c7b537a9 - (u>>1); // gives initial guess y0
        return std::bit_cast<T>(u);
    }else{
        // static_assert(false,"unsupported type");
        HALT;
        return 0;
    }
}

template<floating T>
__fast_inline T fast_sqrt(T fv) {
    return fv * fast_invsqrt(fv);
}

template<floating T>
__fast_inline T fast_rcp(T fv){
    T temp = fast_invsqrt(fv);
    return temp * temp; 
}


__fast_inline float invsqrt(const float number){return fast_invsqrt(number);}
__fast_inline float sqrt(const float number){return fast_sqrt(number);}
__fast_inline float rcp(const float number){return fast_rcp(number);}

__fast_inline double invsqrt(const double number){return fast_invsqrt(number);}
__fast_inline double sqrt(const double number){return fast_sqrt(number);}
__fast_inline double rcp(const double number){return fast_rcp(number);}

}
