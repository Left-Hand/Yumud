#include "rng.hpp"

using namespace ymd;

static void init(){

    #ifdef ENABLE_RNG
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_RNG, ENABLE);
    RNG_Cmd(ENABLE);
    #endif
}

static int update(){

    #ifdef ENABLE_RNG
    while(RNG_GetFlagStatus(RNG_FLAG_DRDY)==RESET);
    return RNG_GetRandomNumber();
    #else
    return 0;
    #endif
}