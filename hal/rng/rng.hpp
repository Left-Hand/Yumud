#pragma once

#include "sys/core/platform.h"

struct Rng{
// public:
//     Rng(){};

//     static void init(){
//         RCC_AHBPeriphClockCmd(RCC_AHBPeriph_RNG, ENABLE);
//         RNG_Cmd(ENABLE);
//     }

//     static int update(){
//         while(RNG_GetFlagStatus(RNG_FLAG_DRDY)==RESET);
//         return RNG_GetRandomNumber();
//     }
};

#ifdef HAVE_RNG
static inline Rng rng
#endif
