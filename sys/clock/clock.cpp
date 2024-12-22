#include "clock.hpp"
#include "sys/core/sdk.h"
#include "sys/core/system.hpp"
#include "sys/debug/debug_inc.h"


#define TICKS_PER_MS (F_CPU / 1000)
#define TICKS_PER_US (TICKS_PER_MS / 1000)

#define MICRO_TRIM 0
#define NANO_TRIM 300

#define TICKS_PER_MS (F_CPU / 1000)
#define TICKS_PER_US (TICKS_PER_MS / 1000)

#ifdef N32G45X
#define M_SYSTICK_CNT SysTick->VAL
#else
#define M_SYSTICK_CNT SysTick->CNT
#define M_SYSTICK_DISER    NVIC_DisableIRQ(SysTicK_IRQn);
#define M_SYSTICK_ENER    NVIC_EnableIRQ(SysTicK_IRQn);
#endif

#define NANO_MUT(x) (((x) * 1000) / TICKS_PER_US)

// extern volatile uint32_t msTick;

volatile uint32_t msTick = 0;


uint32_t millis(void){
  return msTick;
}
uint64_t micros(void){
    M_SYSTICK_DISER;
    uint32_t m = msTick;
    uint32_t ticks = (uint32_t)M_SYSTICK_CNT;
    M_SYSTICK_ENER;

    return (m * 1000 + ticks / TICKS_PER_US);
}

uint64_t nanos(void){
    M_SYSTICK_DISER;
    uint32_t m = msTick;
    uint32_t ticks = (uint32_t)M_SYSTICK_CNT;
    M_SYSTICK_ENER;

    return (m * 1000000 + NANO_MUT(ticks));
}

void delay(const uint32_t ms)
{
  delayMicroseconds(ms * 1000);
}

void delayMicroseconds(const uint32_t us)
{
  uint32_t currentTicks = SysTick->CNT;
  /* Number of ticks per millisecond */
  uint32_t tickPerMs = SysTick->CMP + 1;
  /* Number of ticks to count */
  // uint64_t nbTicks = MAX(us - MICRO_TRIM, 0) * TICKS_PER_US;
  uint32_t nbTicks = us * TICKS_PER_US;
  /* Number of elapsed ticks */
  uint32_t elapsedTicks = 0;
  uint32_t oldTicks = currentTicks;
  do {
    currentTicks = SysTick->CNT;
    // elapsedTicks += (oldTicks < currentTicks) ? tickPerMs + oldTicks - currentTicks :
    //                 oldTicks - currentTicks;

    //increment
    elapsedTicks += (oldTicks <= currentTicks) ? currentTicks - oldTicks :
                    tickPerMs - oldTicks + currentTicks;

    oldTicks = currentTicks;
    } while (nbTicks > elapsedTicks);
}

void delayNanoseconds(uint32_t ns) {
    __IO uint64_t currentTicks = SysTick->CNT;
    /* Number of ticks per millisecond */
    uint64_t tickPerMs = SysTick->CMP + 1;
    /* Number of ticks to count */
    uint64_t nbTicks = NANO_MUT(MAX(ns - NANO_TRIM, 0));
    /* Number of elapsed ticks */
    uint64_t elapsedTicks = 0;
    __IO uint64_t oldTicks = currentTicks;
    do {
        currentTicks = SysTick->CNT;
        // elapsedTicks += (oldTicks < currentTicks) ? tickPerMs + oldTicks - currentTicks :
        //                 oldTicks - currentTicks;

        //increment
        elapsedTicks += (oldTicks <= currentTicks) ? currentTicks - oldTicks :
                        tickPerMs - oldTicks + currentTicks;

        oldTicks = currentTicks;
    } while (nbTicks > elapsedTicks);
}

void Systick_Init(){
    static uint8_t initd = 0;
    if(initd) return;
    initd = 1;
    SysTick->SR  = 0;
    SysTick->CTLR= 0;
    SysTick->CNT = 0;
    SysTick->CMP = TICKS_PER_MS - 1;
    SysTick->CTLR= 0xF;

    NVIC_SetPriority(SysTicK_IRQn,0xFF);
    NVIC_EnableIRQ(SysTicK_IRQn);
}


static std::function<void(void)> cb;


void bindSystickCb(std::function<void(void)> && _cb){
    cb = _cb;
}


void SysTick_Handler(void){
    msTick+=1;
    SysTick->SR = 0;
    Sys::Clock::reCalculateTime();
    // DEBUG_PRINTLN(Sys::t);
    EXECUTE(cb);
}

real_t time(){
    return iq_t(_iq(
        (micros() * (1 << GLOBAL_Q)) / 1000000
    ));
}

#undef TICKS_PER_MS
#undef TICKS_PER_US

#undef M_SYSTICK_CNT
#undef M_SYSTICK_DISER
#undef M_SYSTICK_ENER
