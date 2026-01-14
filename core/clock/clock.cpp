#include <functional>
#include <atomic>

#include "core/math/iq/fixed_t.hpp"
#include "core/sdk.hpp"


#include "clock.hpp"
#include "time.hpp"


static constexpr size_t TICKS_PER_MS = (size_t(F_CPU) / 1000);
static constexpr size_t TICKS_CNTS_PER_US = (TICKS_PER_MS / 1000);

static constexpr size_t MICRO_TRIM = 0;
static constexpr size_t NANO_TRIM = 300;



#define M_SYSTICK_DISER    NVIC_DisableIRQ(SysTicK_IRQn);
#define M_SYSTICK_ENER    NVIC_EnableIRQ(SysTicK_IRQn);


[[nodiscard]] static __fast_inline constexpr 
uint32_t ticks_to_nanos(const uint32_t num_ticks){
    return (((num_ticks) * 1000) / TICKS_CNTS_PER_US);
}
[[nodiscard]] static __fast_inline
uint32_t get_systick_cnt(){
    return SysTick->CNT;
}

[[nodiscard]] static __fast_inline
uint32_t get_systick_cmp(){
    return SysTick->CMP;
}


static std::function<void(void)> SYSTICK_HANDLER_ = nullptr;
static volatile uint32_t MILLIS_CNT_ = 0;


static __fast_inline uint32_t millis_cnt(void){
    // return MILLIS_CNT_.load(std::memory_order_seq_cst);
    return MILLIS_CNT_;
}

static __fast_inline uint64_t micros_cnt(void){
    // M_SYSTICK_DISER;
    const volatile uint32_t systick_cnt = static_cast<uint32_t>(get_systick_cnt());
    // const auto ctlr = SysTick->CTLR;
    // if(ctlr & 0x10000){MILLIS_CNT_.fetch_add(1);}
    const volatile uint64_t base = static_cast<uint64_t>(millis_cnt()) * 1000u;
    // M_SYSTICK_ENER;

    // return (base + static_cast<uint32_t>(systick_cnt / TICKS_CNTS_PER_US));
    return (base + static_cast<uint32_t>((systick_cnt * 455) >> 16));
}

static __fast_inline uint64_t nanos_cnt(void){
    #if 0
    M_SYSTICK_DISER;
    const uint64_t base = static_cast<uint64_t>(millis_cnt()) * 1000u;
    const uint32_t systick_cnt = static_cast<uint32_t>(get_systick_cnt());
    M_SYSTICK_ENER;

    return (base + ticks_to_nanos(systick_cnt));
    #else
    return 0;
    #endif
}



static __fast_inline void delay_us(const uint32_t us){
    uint32_t current_ticks = get_systick_cnt();
    /* Number of systick_cnt per millisecond */
    uint32_t ticks_per_ms = get_systick_cmp() + 1;
    /* Number of systick_cnt to count */
    // uint64_t num_ticks = MAX(us - MICRO_TRIM, 0) * TICKS_CNTS_PER_US;
    uint32_t num_ticks = us * TICKS_CNTS_PER_US;
    /* Number of elapsed systick_cnt */
    uint32_t elapsed_ticks = 0;
    uint32_t old_ticks = current_ticks;
    do {
        current_ticks = get_systick_cnt();
        // elapsed_ticks += (old_ticks < current_ticks) ? ticks_per_ms + old_ticks - current_ticks :
        //                 old_ticks - current_ticks;

    //increment
    elapsed_ticks += (old_ticks <= current_ticks) ? current_ticks - old_ticks :
                    ticks_per_ms - old_ticks + current_ticks;

    old_ticks = current_ticks;
    } while (num_ticks > elapsed_ticks);
}

static __fast_inline void delay_ns(uint32_t ns) {
    volatile uint64_t current_ticks = get_systick_cnt();
    /* Number of systick_cnt per millisecond */
    uint64_t ticks_per_ms = get_systick_cmp() + 1;
    /* Number of systick_cnt to count */
    uint64_t num_ticks = ticks_to_nanos(MAX(ns - NANO_TRIM, 0));
    /* Number of elapsed systick_cnt */
    uint64_t elapsed_ticks = 0;
    volatile uint64_t old_ticks = current_ticks;
    do {
        current_ticks = get_systick_cnt();
        // elapsed_ticks += (old_ticks < current_ticks) ? ticks_per_ms + old_ticks - current_ticks :
        //                 old_ticks - current_ticks;

        //increment
        elapsed_ticks += (old_ticks <= current_ticks) ? current_ticks - old_ticks :
                        ticks_per_ms - old_ticks + current_ticks;

        old_ticks = current_ticks;
    } while (num_ticks > elapsed_ticks);
}

static void delay_ms(const uint32_t ms){
    delay_us(ms * 1000);
}

namespace ymd::clock{

Milliseconds millis(void){
    return Milliseconds(millis_cnt());
}

Microseconds micros(void){
    __disable_irq();
    const auto ret = Microseconds(micros_cnt());
    __enable_irq();
    return ret;
}
Nanoseconds nanos(void){
    return Nanoseconds(nanos_cnt());
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


void set_systick_handler(std::function<void(void)> && cb){
    SYSTICK_HANDLER_ = std::move(cb);
}



static consteval double sepow(const double base, const size_t times){
    double ret = 1;
    for(size_t i = 0; i < times; i++){
        ret *= base;
    }
    return ret;
}

uq16 time(){

    struct Dump{
        uint64_t l15:15;
        uint64_t m15:15;
        uint64_t h31:31;
        uint64_t unused:3;//精度足以万年 可以舍弃3位
    };

    static_assert(sizeof(Dump) == sizeof(uint64_t));

    const Dump dump = std::bit_cast<Dump>(uint64_t(micros().count()));

    return 
        + uq16::from_bits((static_cast<uint32_t>(dump.l15) << 16) / 1000000)
        + uq16((1 << 15) / 1e6) * static_cast<uint32_t>(dump.m15)
        + uq16((1 << 30) / 1e6) * static_cast<uint32_t>(dump.h31)
        ;
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
    __disable_irq();
    MILLIS_CNT_++;
    SysTick->SR = 0;
    __enable_irq();
    EXECUTE(SYSTICK_HANDLER_);
}