#include "clock.h"

static std::function<void(void)> cb;

void bindSystickCb(std::function<void(void)> && _cb){
    cb = _cb;
}

extern "C"{
void SysTick_Handler(void)
{
    msTick+=1;
    SysTick->SR = 0;
    Sys::Clock::reCalculateTime();
    EXECUTE(cb);
}

}