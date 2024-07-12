#ifndef __FAST_SQRT_H__

#define __FAST_SQRT_H__

#include "sys/core/platform.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __has_include
    #if __has_include(<cmath>)
        #include <cmath>
        #define __STD_MATH_EXIST
    #endif
    #if __has_include(<math.h>)
        #include <math.h>
        #define __STD_MATH_EXIST
    #endif
#endif

static __fast_inline float int2float(int number) {
    union {
        int i;
        float f;
    } converter;
    converter.i = number;
    return converter.f;
}

static __fast_inline int float2int(float number) {
    union {
        int i;
        float f;
    } converter;
    converter.f = number;
    return converter.i;
}


__fast_inline float fast_invsqrt(float number){
    float xhalf = 0.5f*number;
    int i = float2int(number);
    i = 0x5f375a86 - (i>>1); // gives initial guess y0
    number = int2float(i);
    number = number*(1.5f-xhalf*number*number); // Newton step, repeating increases accuracy
    return number;
}

__fast_inline float fast_sqrt(float number) {

    float x = number * 0.5f;
    float y = number;
    int i = float2int(y);
    i = 0x5f3759df - ( i >> 1 );
    y = int2float(i);
    y = y * (1.5f - ( x * y * y ));
    y = y * (1.5f - ( x * y * y ));
    return number * y;
}

#if defined(__cplusplus) && (!defined(__STD_MATH_EXIST))
__fast_inline float std::invsqrt(const float & number){return fast_invsqrt(number);}
__fast_inline float sqrt(const float & number){return fast_sqrt(number);}
#endif

#ifdef __cplusplus
}
#endif

#endif