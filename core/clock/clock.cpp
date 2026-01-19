#include <functional>
#include <atomic>

#include "core/math/iq/fixed_t.hpp"
#include "core/utils/scope_guard.hpp"
#include "core/clock/clock.hpp"
#include "core/clock/time.hpp"
#include "core/sdk.hpp"


static constexpr size_t TICK_CNTS_PER_MS = (size_t(F_CPU) / 1000);
constexpr uint32_t TICK_CNTS_PER_US = (TICK_CNTS_PER_MS / 1000);

static constexpr size_t MICRO_TRIM = 0;
static constexpr size_t NANO_TRIM = 300;

static std::function<void(void)> SYSTICK_CALLBACK_ = nullptr;

namespace {

#if 0
struct alignas(4) [[nodiscard]] MillisCounter{
    alignas(4) std::atomic<uint32_t> count_;

    // 若当前在高优先级中断中发生了还未响应的systick中断
    // 可以手动检查一次是否有未处理的定时器溢出事件并帮助更新毫秒计数
    // 避免systick计数器和毫秒数撕裂的情况出现 
    // !非常重要
    __attribute__((always_inline)) uint32_t poll_and_get(){
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
#else
struct alignas(4) [[nodiscard]] MillisCounter{
    uint64_t count_;

    // 若当前在高优先级中断中发生了还未响应的systick溢出事件
    // 可以手动检查一次是否有未处理的systick溢出事件并帮助更新毫秒计数
    // 避免systick计数器和毫秒数撕裂的情况出现 
    // !非常重要
    __attribute__((always_inline)) uint64_t poll_and_get(){
        __disable_irq();
        __disable_irq();
        const auto guard = ymd::make_scope_guard([]{__enable_irq();});
        //判断是否发生了计数器溢出
        if(SysTick->SR) [[likely]] {
            //清除标志位
            SysTick->SR = 0;
            count_++;
        }
        return count_;
    }
};
#endif



}
static MillisCounter MILLIS_COUNTER_ = MillisCounter{0};



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


[[nodiscard]] static __attribute__((always_inline)) constexpr 
uint32_t _ticks_to_nanos(const uint32_t num_ticks){
    return (((num_ticks) * 1000) / TICK_CNTS_PER_US);
}

[[nodiscard]] static __attribute__((always_inline)) constexpr 
uint32_t _ticks_to_micros(const uint32_t num_ticks){

    constexpr uint32_t MUL_FACTOR = 65536 / TICK_CNTS_PER_US;
    return (static_cast<uint32_t>((num_ticks * MUL_FACTOR) >> 16));
}

[[nodiscard]] static
uint32_t _get_systick_cnt(){
    //只获取低32位 因为不会计数到那么大 两个32位不具有原子性
    // return static_cast<uint32_t>(*reinterpret_cast<volatile uint32_t *>(&(SysTick->CNT)));
    return static_cast<uint32_t>(SysTick->CNT);
}



[[nodiscard]] static 
uint64_t _get_millis_cnt(void){
    return MILLIS_COUNTER_.poll_and_get();
}

[[nodiscard]] static 
uint64_t _get_micros_cnt(void){
    const uint64_t millis_cnt = MILLIS_COUNTER_.poll_and_get();
    const uint32_t systick_cnt = static_cast<uint32_t>(_get_systick_cnt());
    const uint64_t base = static_cast<uint64_t>(millis_cnt) * 1000u;

    return (base + _ticks_to_micros(systick_cnt));

}

[[nodiscard]] static
uint64_t _get_nanos_cnt(void){
    const uint64_t millis_cnt = MILLIS_COUNTER_.poll_and_get();
    const uint32_t systick_cnt = static_cast<uint32_t>(_get_systick_cnt());
    const uint64_t base = static_cast<uint64_t>(millis_cnt) * 1000000u;

    return (base + _ticks_to_nanos(systick_cnt));
}

// __attribute__((always_inline))  static
static
void _delay_ticks(const uint64_t duration_ticks){
    /* Number of elapsed systick_cnt */
    
    uint32_t last_ticks = _get_systick_cnt();
    uint32_t now_ticks = last_ticks;
    
    volatile uint64_t elapsed_ticks = 0;
    do {
        now_ticks = _get_systick_cnt();

        if(now_ticks >= last_ticks){
            elapsed_ticks += static_cast<uint64_t>(now_ticks - last_ticks);
        }else{
            elapsed_ticks += static_cast<uint64_t>(TICK_CNTS_PER_MS - last_ticks + now_ticks);
        }
        last_ticks = now_ticks;
    } while (duration_ticks > elapsed_ticks);
}

static void _delay_us(const uint64_t num_us){
    const uint64_t duration_ticks = num_us * TICK_CNTS_PER_US;
    _delay_ticks(duration_ticks);
}

static void _delay_ns(const uint64_t num_ns) {
    const uint64_t duration_ticks = _ticks_to_nanos(MAX(num_ns - NANO_TRIM, 0));
    _delay_ticks(duration_ticks);
}

static void _delay_ms(const uint64_t num_ms){
    const uint64_t duration_ticks = num_ms * TICK_CNTS_PER_MS;
    _delay_ticks(duration_ticks);
}

namespace ymd::clock{

Milliseconds millis(void){
    return Milliseconds(_get_millis_cnt());
}

Microseconds micros(void){
    return Microseconds(_get_micros_cnt());
}

Nanoseconds nanos(void){
    return Nanoseconds(_get_nanos_cnt());
}

void delay(Milliseconds ms){
    _delay_ms(ms.count());
}

void delay(Microseconds us){
    _delay_us(us.count());
}

void delay(Nanoseconds ns){
    _delay_ns(ns.count());
}


void set_systick_handler(std::function<void(void)> && cb){
    SYSTICK_CALLBACK_ = std::move(cb);
}

namespace {
//为了避免64位计算在32位处理器上引入的除法开销 将64位的微秒刻分成三个部分
struct MicrosDump{
    uint64_t low_15:15;
    uint64_t middle_15:15;
    uint64_t high_31:31;
    uint64_t unused:3;//精度足以万年 可以舍弃3位
};

//经过测试这段代码在120年的时间刻度内能保证可靠性
static constexpr math::fixed_t<32, uint64_t> micros_to_seconds(uint64_t micros_count){
    static_assert(sizeof(MicrosDump) == sizeof(uint64_t));
    const MicrosDump micros_dump = std::bit_cast<MicrosDump>(micros_count);
    uint64_t result_bits = 0;
    
    {
        uint64_t value = static_cast<uint64_t>(micros_dump.low_15);
        constexpr uint64_t FACTOR = (std::numeric_limits<uint64_t>::max()) / 1000000ULL;
        uint64_t scaled_value = (value * FACTOR) >> 32;

        result_bits += scaled_value;
    }
    
    {
        constexpr uint64_t FACTOR = math::fixed_t<32, uint64_t>((1 << 15) / 1e6).to_bits();
        uint64_t value = static_cast<uint64_t>(micros_dump.middle_15);
        uint64_t scaled_value = (value * FACTOR);
        result_bits += scaled_value;
    }
    
    {
        constexpr uint64_t FACTOR = math::fixed_t<32, uint64_t>((1 << 30) / 1e6).to_bits() >> 11;
        static_assert(FACTOR < std::numeric_limits<uint32_t>::max());
        uint64_t value = static_cast<uint64_t>(micros_dump.high_31);
        uint64_t scaled_value = (value * FACTOR) << 11;
        result_bits += scaled_value;
    }

    return math::fixed_t<32, uint64_t>::from_bits(result_bits);
}

static_assert(micros_to_seconds(1000000).to_bits() == 1.0 * (1Ull << 32));
static_assert(micros_to_seconds(uint64_t(1E8)).to_bits() == 1E2 * (1Ull << 32));
static_assert(micros_to_seconds(uint64_t(4E8)).to_bits() == 4E2 * (1Ull << 32));
static_assert(micros_to_seconds(uint64_t(7E8)).to_bits() == 7E2 * (1Ull << 32));
static_assert(micros_to_seconds(uint64_t(1E12)).to_bits() == 1E6 * (1Ull << 32));
static_assert(micros_to_seconds(uint64_t(1E15)).to_bits() == 1E9 * (1Ull << 32));
static_assert(micros_to_seconds(uint64_t(4E15)).to_bits() == 4E9 * (1Ull << 32));
}


math::fixed_t<32, uint64_t> seconds_precious(){
    return micros_to_seconds(micros().count());
}

math::fixed_t<16, uint32_t> seconds(){


    static_assert(sizeof(MicrosDump) == sizeof(uint64_t));

    const MicrosDump micros_dump = std::bit_cast<MicrosDump>(uint64_t(micros().count()));

    uint32_t result_bits = 0;
    {
        uint64_t value = static_cast<uint64_t>(micros_dump.low_15);
        constexpr uint64_t FACTOR = (std::numeric_limits<uint64_t>::max()) / 1000000ULL;
        uint64_t scaled_value = (value * FACTOR) >> 48;

        result_bits += scaled_value;
    }
    
    auto result = math::fixed_t<16, uint32_t>::from_bits(result_bits);
    return 
        result
        + math::fixed_t<16, uint32_t>((1 << 15) / 1e6) * static_cast<uint32_t>(micros_dump.middle_15)
        + math::fixed_t<16, uint32_t>((1 << 30) / 1e6) * static_cast<uint32_t>(micros_dump.high_31)
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