#include "clock.h"
#include "time.hpp"

#include "core/math/iq/iq_t.hpp"
#include <functional>

#include "core/sdk.h"

using namespace ymd;

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
volatile uint64_t micros_base = 0;


uint32_t millis(void){
  return msTick;
}
uint64_t micros(void){
    // M_SYSTICK_DISER;
    // static uint64_t last_m = 0;
    // uint32_t m = msTick;
    // uint32_t ticks = (uint32_t)M_SYSTICK_CNT;
    // M_SYSTICK_ENER;

    // uint64_t new_m(m * 1000 + ticks / TICKS_PER_US);
    // if(new_m < last_m){
    //     return last_m = new_m + 1000;
    // }else{
    //     return last_m = new_m;
    // }

    M_SYSTICK_DISER;
    // const uint32_t m = msTick;
    const uint32_t base = micros_base;
    const uint32_t ticks = (uint32_t)M_SYSTICK_CNT;
    M_SYSTICK_ENER;

    return (base + ticks / TICKS_PER_US);
}

uint64_t nanos(void){
    M_SYSTICK_DISER;
    const uint32_t base = micros_base;
    const uint32_t ticks = (uint32_t)M_SYSTICK_CNT;
    M_SYSTICK_ENER;

    return (base + NANO_MUT(ticks));
}

void delay(const uint32_t ms){
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
    SysTick->SR  = 0;
    SysTick->CTLR= 0;
    SysTick->CNT = 0;
    SysTick->CMP = TICKS_PER_MS - 1;
    SysTick->CTLR= 0xF;

    NVIC_SetPriority(SysTicK_IRQn,0xFF);
    NVIC_EnableIRQ(SysTicK_IRQn);
}


static std::function<void(void)> systick_cb;

void bindSystickCb(std::function<void(void)> && _cb){
    systick_cb = _cb;
}


void SysTick_Handler(void){
    msTick += 1;
    micros_base += 1000;

    SysTick->SR = 0;
    EXECUTE(systick_cb);
}

static consteval double sepow(const double base, const size_t times){
    double ret = 1;
    for(size_t i = 0; i < times; i++){
        ret *= base;
    }
    return ret;
}

real_t ymd::time(){
    if constexpr(is_fixed_point_v<real_t>){
        union Depart{
            uint64_t res64;
            struct{
                uint64_t l15:15;
                uint64_t m15:15;
                uint64_t h31:31;
                uint64_t unused:3;//精度足以万年 可以舍弃3位
            };
        };

        const Depart microsec = Depart{.res64 = micros()};

        return 
            + real_t{_iq<16>::from_i32((int(microsec.l15) << 16) / 1000000)} 
            + real_t{int(microsec.m15)} * real_t(sepow(2, 15) / sepow(10, 6))
            + real_t{int(microsec.h31)} * real_t(sepow(2, 30) / sepow(10, 6))
            ;
    }else{

        HALT;
        return 0;
    }

}

#undef TICKS_PER_MS
#undef TICKS_PER_US

#undef M_SYSTICK_CNT
#undef M_SYSTICK_DISER
#undef M_SYSTICK_ENER
