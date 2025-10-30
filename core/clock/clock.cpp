#include "clock.hpp"
#include "time.hpp"

#include "core/math/iq/fixed_t.hpp"
#include <functional>

#include "core/sdk.hpp"

static constexpr size_t TICKS_PER_MS = (F_CPU / 1000);
static constexpr size_t TICKS_PER_US = (TICKS_PER_MS / 1000);

static constexpr size_t MICRO_TRIM = 0;
static constexpr size_t NANO_TRIM = 300;

#ifdef N32G45X
#define M_SYSTICK_CNT SysTick->VAL
#else
#define M_SYSTICK_CNT SysTick->CNT
#define M_SYSTICK_DISER    NVIC_DisableIRQ(SysTicK_IRQn);
#define M_SYSTICK_ENER    NVIC_EnableIRQ(SysTicK_IRQn);
#endif

#define NANO_MUT(x) (((x) * 1000) / TICKS_PER_US)


static volatile uint32_t msTick = 0;
static volatile uint64_t micros_base = 0;


static __fast_inline uint32_t millis_impl(void){
    return msTick;
}

static __fast_inline uint64_t micros_impl(void){
    M_SYSTICK_DISER;
    const uint32_t base = micros_base;
    const uint32_t ticks = (uint32_t)M_SYSTICK_CNT;
    M_SYSTICK_ENER;

    return (base + (ticks / TICKS_PER_US));
    // return base * 10;
}

static __fast_inline uint64_t nanos_impl(void){
    M_SYSTICK_DISER;
    const uint32_t base = micros_base;
    const uint32_t ticks = (uint32_t)M_SYSTICK_CNT;
    M_SYSTICK_ENER;

    return (base + NANO_MUT(ticks));
}



static __fast_inline void delay_us(const uint32_t us){
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

static __fast_inline void delay_ns(uint32_t ns) {
    volatile uint64_t currentTicks = SysTick->CNT;
    /* Number of ticks per millisecond */
    uint64_t tickPerMs = SysTick->CMP + 1;
    /* Number of ticks to count */
    uint64_t nbTicks = NANO_MUT(MAX(ns - NANO_TRIM, 0));
    /* Number of elapsed ticks */
    uint64_t elapsedTicks = 0;
    volatile uint64_t oldTicks = currentTicks;
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

static void delay_ms(const uint32_t ms){
    delay_us(ms * 1000);
}


static std::function<void(void)> systick_cb;

namespace ymd::clock{



Milliseconds millis(void){
    return Milliseconds(millis_impl());
}

Microseconds micros(void){
    return Microseconds(micros_impl());
}
Nanoseconds nanos(void){
    return Nanoseconds(nanos_impl());
}

void delay(Milliseconds ms){
    delay_ms(ms.count());
}

void delay(Microseconds us){
    delay_us(us.count());
}

void delay(Nanoseconds ns){
    delay_ns(ns.count());
}


void bindSystickCb(std::function<void(void)> && cb){
    systick_cb = std::move(cb);
}



static consteval double sepow(const double base, const size_t times){
    double ret = 1;
    for(size_t i = 0; i < times; i++){
        ret *= base;
    }
    return ret;
}

real_t time(){
    if constexpr(is_fixed_point_v<real_t>){
        struct Depart{
            uint64_t l15:15;
            uint64_t m15:15;
            uint64_t h31:31;
            uint64_t unused:3;//精度足以万年 可以舍弃3位
        };

        // const Depart microsec = Depart{.res64 = uint64_t(micros().count())};
        const Depart microsec = std::bit_cast<Depart>(uint64_t(micros().count()));

        return 
            + real_t{fixed_t<16, int32_t>::from_bits((int(microsec.l15) << 16) / 1000000)} 
            + real_t{int(microsec.m15)} * real_t(sepow(2, 15) / sepow(10, 6))
            + real_t{int(microsec.h31)} * real_t(sepow(2, 30) / sepow(10, 6))
            ;
    }else{

        HALT;
        return 0;
    }

}

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

void SysTick_Handler(void){
    msTick += 1;
    micros_base += 1000;

    SysTick->SR = 0;
    EXECUTE(systick_cb);
}