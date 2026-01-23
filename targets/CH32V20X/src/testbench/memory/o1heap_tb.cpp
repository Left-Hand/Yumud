#include "src/testbench/tb.h"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/scope_guard.hpp"
#include "core/utils/zero.hpp"
#include "core/async/timer.hpp"


#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"


#include "drivers/Proximeter/STL06N/stl06n.hpp"

#include <ranges>

#include "core/mem/o1heap/o1heap_alloc.hpp"
#include "core/string/view/string_view.hpp"


using namespace ymd;
using namespace lib_o1heap;

[[nodiscard]] __no_inline static constexpr uint32_t  clzsi2(uint32_t x){
    // under both the University of Illinois "BSD-Like" license and the MIT license
    // https://github.com/m-labs/compiler-rt-lm32/blob/master/lib/clzsi2.c

    #if 0
    int32_t t = ((x & 0xFFFF0000) == 0) << 4;  /* if (x is small) t = 16 else 0 */
    x >>= 16 - t;      /* x = [0 - 0xFFFF] */
    uint32_t r = t;       /* r = [0, 16] */
    /* return r + clz(x) */
    t = ((x & 0xFF00) == 0) << 3;
    x >>= 8 - t;       /* x = [0 - 0xFF] */
    r += t;            /* r = [0, 8, 16, 24] */
    /* return r + clz(x) */
    t = ((x & 0xF0) == 0) << 2;
    x >>= 4 - t;       /* x = [0 - 0xF] */
    r += t;            /* r = [0, 4, 8, 12, 16, 20, 24, 28] */
    /* return r + clz(x) */
    t = ((x & 0xC) == 0) << 1;
    x >>= 2 - t;       /* x = [0 - 3] */
    r += t;            /* r = [0 - 30] and is even */

    return r + ((2 - x) & -((x & 2) == 0));
    #else
    if (x == 0) return 32;
    
    int n = 0;
    if (x <= 0x0000FFFF) { n += 16; x <<= 16; }
    if (x <= 0x00FFFFFF) { n += 8;  x <<= 8;  }
    if (x <= 0x0FFFFFFF) { n += 4;  x <<= 4;  }
    if (x <= 0x3FFFFFFF) { n += 2;  x <<= 2;  }
    if (x <= 0x7FFFFFFF) { n += 1;  x <<= 1;  }
    return n;
    #endif
}


struct alignas(16) Object{
    using Self = Object;
    uint8_t name[4];
    uint32_t x;
    uint32_t y;
    uint32_t length;

    friend OutputStream & operator << (OutputStream & os, const Self & self){
        return os << char(self.name[0]) << os.splitter() 
        << self.x <<os.splitter() 
        << self.y <<os.splitter() 
        << self.length;
    }
};
void o1heap_main(){
    #if defined(CH32V20X)
    auto & UART = hal::usart2;
    UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking
    });
    #elif defined(CH32V30X)
    auto & UART = hal::uart6;
    UART.init({
        .remap = hal::UART6_REMAP_PC0_PC1,
        .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking
    });
    #endif
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);
    DEBUGGER.no_fieldname(EN);


    // std::array<uint8_t, 8192> buffer{};
    // auto buffer = std::make_unique<std::array<uint8_t, 8192>>();
    // static constexpr size_t buffer_size = 2048;
    static constexpr size_t buffer_size = 8192;
    auto buffer = std::make_unique<uint8_t[]>(buffer_size);
    {
        ymd::mem::o1heap::O1HeapMemoryResource resource(std::span(buffer.get(), buffer_size));
        
        // 使用 PMR 容器
        // std::pmr::vector<uint8_t> vec(&resource);



        std::pmr::vector<Object> vec(&resource);
        vec.reserve(100);

        const auto elapsed_us = measure_total_elapsed_us(
            [&]{
                for(size_t i = 0; i < 100; i++){
                    vec.push_back(Object{
                        .name = { 'O', 'b', 'j', 'e' },
                        .x = i * 10,
                        .y = i * 20,
                        .length = i * 30
                    });
                }
            }
        );
        vec.shrink_to_fit();
        
        DEBUGGER << "Vector size: " << vec.size() << std::endl;
        for (const auto& val : vec | std::views::take(30)) {
            DEBUGGER << val << " ";
        }
        DEBUGGER << std::endl;
        
        // 字符串也使用相同的内存池
        std::pmr::string str("Hello, PMR with O1Heap!", &resource);
        DEBUGGER << "String: " << str.length() << StringView(str) << std::endl;
        
        // 获取诊断信息
        auto diagnostics = resource.get_diagnostics();
        DEBUGGER << "Peak allocated: " << diagnostics.peak_allocated << " bytes" << std::endl;
        DEBUGGER << elapsed_us << std::endl;
        __builtin_trap();
    }
}