#ifndef __FAST_SQRT_H__

#define __FAST_SQRT_H__

#include "../src/defines/comm_defs.h"

#ifdef __cplusplus
extern "C"{
#endif

__fast_inline float int2float(int number) {
    union {
        int i;
        float f;
    } converter;
    converter.i = number;
    return converter.f;
}

__fast_inline int float2int(float number) {
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


#ifdef __cplusplus
}
#endif

#endif