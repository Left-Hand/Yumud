#include <functional>
#include <atomic>

#include "core/math/iq/fixed_t.hpp"
#include "core/sdk.hpp"


#include "clock.hpp"
#include "time.hpp"


static constexpr size_t TICK_CNTS_PER_MS = (size_t(F_CPU) / 1000);
constexpr uint32_t TICK_CNTS_PER_US = (TICK_CNTS_PER_MS / 1000);

static constexpr size_t MICRO_TRIM = 0;
static constexpr size_t NANO_TRIM = 300;

#if 0
#ifdef __riscv
namespace riscv_systick_def{
struct R32_STK_CTLR{
    uint32_t STE:1;
    uint32_t STIE:1;
    uint32_t STCLK:1;
    uint32_t STRE:1;
    uint32_t MODE:1;
    uint32_t INIT:1;
    uint32_t :25;
    uint32_t SWIE:1;
};
}

#elif defined(__arm__)
namespace arm_systick_def{
}

#error "not supported yet"
#else
#error "not supported yet"
#endif
#endif

#define __DEF_DISABLE_SYSTICK_IRQ    NVIC_DisableIRQ(SysTicK_IRQn);
#define __DEF_ENABLE_SYSTICK_IRQ    NVIC_EnableIRQ(SysTicK_IRQn);


[[nodiscard]] static __fast_inline constexpr 
uint32_t _ticks_to_nanos(const uint32_t num_ticks){
    return (((num_ticks) * 1000) / TICK_CNTS_PER_US);
}

[[nodiscard]] static __fast_inline constexpr 
uint32_t _ticks_to_micros(const uint32_t num_ticks){

    constexpr uint32_t MUL_FACTOR = 65536 / TICK_CNTS_PER_US;
    return (static_cast<uint32_t>((num_ticks * MUL_FACTOR) >> 16));
}

[[nodiscard]] static __fast_inline
uint32_t get_systick_cnt(){
    //只获取低32位 因为不会计数到那么大 两个32位不具有原子性
    // return static_cast<uint32_t>(*reinterpret_cast<volatile uint32_t *>(&(SysTick->CNT)));
    return static_cast<uint32_t>(SysTick->CNT);
}


static std::function<void(void)> SYSTICK_CALLBACK_ = nullptr;

namespace {
struct alignas(4) [[nodiscard]] MillisCounter{
    alignas(4) std::atomic<uint32_t> count_;

    // 若当前在高优先级中断中发生了还未响应的systick中断
    // 可以手动检查一次是否有未处理的定时器溢出事件并帮助更新毫秒计数
    // 避免systick计数器和毫秒数撕裂的情况出现 
    // !非常重要
    __fast_inline uint32_t poll_and_get(){
        const uint32_t now_sr = SysTick->SR;
        //判断是否发生了计数器溢出
        if(now_sr) [[likely]] {
            __disable_irq();
            __disable_irq();
            //清除标志位
            SysTick->SR = 0;
            count_.fetch_add(1, std::memory_order_relaxed);
            __enable_irq();
        }
        return count_.load(std::memory_order_relaxed);
    }
};
}
static MillisCounter MILLIS_COUNTER_ = MillisCounter{0};


static __fast_inline uint32_t get_millis_cnt(void){
    return MILLIS_COUNTER_.poll_and_get();
}

static __fast_inline uint64_t get_micros_cnt(void){
    const uint32_t millis_cnt = MILLIS_COUNTER_.poll_and_get();
    const uint32_t systick_cnt = static_cast<uint32_t>(get_systick_cnt());
    const uint64_t base = static_cast<uint64_t>(millis_cnt) * 1000u;

    return (base + _ticks_to_micros(systick_cnt));

}

static __fast_inline uint64_t nanos_cnt(void){
    const uint32_t millis_cnt = MILLIS_COUNTER_.poll_and_get();
    const uint32_t systick_cnt = static_cast<uint32_t>(get_systick_cnt());
    const uint64_t base = static_cast<uint64_t>(millis_cnt) * 1000000u;

    return (base + _ticks_to_nanos(systick_cnt));
}

static void delay_ticks(const uint64_t duration_ticks){
    /* Number of elapsed systick_cnt */
    
    uint32_t last_ticks = get_systick_cnt();
    uint32_t now_ticks = last_ticks;
    
    volatile uint64_t elapsed_ticks = 0;
    do {
        now_ticks = get_systick_cnt();

        if(now_ticks >= last_ticks){
            elapsed_ticks += static_cast<uint64_t>(now_ticks - last_ticks);
        }else{
            elapsed_ticks += static_cast<uint64_t>(TICK_CNTS_PER_MS - last_ticks + now_ticks);
        }
        last_ticks = now_ticks;
    } while (duration_ticks > elapsed_ticks);
}

static void delay_us(const uint64_t num_us){
    const uint64_t duration_ticks = num_us * TICK_CNTS_PER_US;
    delay_ticks(duration_ticks);
}

static void delay_ns(const uint64_t num_ns) {
    const uint64_t duration_ticks = _ticks_to_nanos(MAX(num_ns - NANO_TRIM, 0));
    delay_ticks(duration_ticks);
}

static void delay_ms(const uint64_t num_ms){
    const uint64_t duration_ticks = num_ms * TICK_CNTS_PER_MS;
    delay_ticks(duration_ticks);
}

namespace ymd::clock{

Milliseconds millis(void){
    return Milliseconds(get_millis_cnt());
}

Microseconds micros(void){
    const auto ret = Microseconds(get_micros_cnt());
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
    SYSTICK_CALLBACK_ = std::move(cb);
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
    SysTick->CMP = TICK_CNTS_PER_MS - 1;
    SysTick->CTLR= 0xF;

    NVIC_SetPriority(SysTicK_IRQn,0xFF);
    NVIC_EnableIRQ(SysTicK_IRQn);
}


extern "C"{
__interrupt
void SysTick_Handler(void){
    //这里返回到了获取的毫秒数 但是我们暂时不用
    (void)MILLIS_COUNTER_.poll_and_get();
    if(SYSTICK_CALLBACK_) SYSTICK_CALLBACK_();
}
}