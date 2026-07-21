#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/utils/default.hpp"
#include "core/async/timer.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/conn/spi/hw_singleton.hpp"
#include "hal/dma/dma.hpp"


// 测试1：compare_exchange_strong - 顺序一致性
__no_inline void test_strong_seq_cst(std::atomic<int>& atomic, int expected, int desired) {
    atomic.compare_exchange_strong(expected, desired);
}

// 测试2：compare_exchange_strong - 指定内存序
__no_inline void test_strong_acq_rel(std::atomic<int>& atomic, int expected, int desired) {
    atomic.compare_exchange_strong(expected, desired, 
                                   std::memory_order_acq_rel,
                                   std::memory_order_acquire);
}

// 测试3：compare_exchange_weak - 循环重试（典型用法）
__no_inline void test_weak_loop(std::atomic<int>& atomic, int desired) {
    int expected = atomic.load();
    while (!atomic.compare_exchange_weak(expected, desired,
                                         std::memory_order_relaxed,
                                         std::memory_order_relaxed)) {
        // 失败时 expected 已被更新，直接重试
    }
}

// 测试4：不同数据类型 - uint32_t (如果是RV64，看是否用AMOCAS.D)
__no_inline void test_uint32(std::atomic<uint32_t>& atomic, uint32_t expected, uint32_t desired) {
    atomic.compare_exchange_strong(expected, desired);
}


using namespace ymd;


#define DBG_UART hal::usart2

void cas_main(){
    DBG_UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576000),
        .rx_strategy = CommStrategy::Dma,
        .tx_strategy = CommStrategy::Blocking,
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.build_config()
        .set_eps(4)
        .set_splitter(",")
        .no_brackets(EN)
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();
    // DEBUGGER.force_sync(EN);


    clock::delay(2ms);

    std::atomic<int> a{10};
    std::atomic<uint32_t> b{100};
    
    test_strong_seq_cst(a, 10, 20);
    test_strong_acq_rel(a, 20, 30);
    test_weak_loop(a, 40);
    test_uint32(b, 100, 200);
    
    while(true){
        DEBUG_PRINTLN(clock::millis());
    }

}