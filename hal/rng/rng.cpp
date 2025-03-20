#include "rng.hpp"
#include "core/sdk.h"

using namespace ymd::hal;

void Rng::init(){

    #ifdef ENABLE_RNG
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_RNG, ENABLE);
    RNG_Cmd(ENABLE);
    #endif
}

int Rng::update(){

    #ifdef ENABLE_RNG
    while(RNG_GetFlagStatus(RNG_FLAG_DRDY)==RESET);
    return RNG_GetRandomNumber();
    #else
    return 0;
    #endif
}