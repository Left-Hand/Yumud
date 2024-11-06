#include "clock.hpp"
#include "sys/core/system.hpp"

static std::function<void(void)> cb;

namespace Sys{
    real_t t;
};

void bindSystickCb(std::function<void(void)> && _cb){
    cb = _cb;
}

extern "C"{
void SysTick_Handler(void)
{
    msTick+=1;
    SysTick->SR = 0;
    Sys::t.value = _iq((msTick * (1 << GLOBAL_Q)) / 1000);
    Sys::Clock::reCalculateTime();
    EXECUTE(cb);
}



}