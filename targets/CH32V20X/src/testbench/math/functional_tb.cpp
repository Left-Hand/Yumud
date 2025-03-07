#include "src/testbench/tb.h"
#include "sys/debug/debug.hpp"

#include "sys/math/real.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "utils/rustlike/Result.hpp"
#include <bitset>
#include <ranges>


using namespace ymd;

struct FoundInfo{
    uint8_t addr;
    uint max_bbaud;
};


#define UART uart2

namespace ranges = std::ranges;

void functional_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.forceSync();
    
    // auto process_address = [](uint8_t addr) -> Option<FoundInfo> {
    //     // return 
    //     //     .map([addr](auto baud) { return FoundInfo{addr, baud}; })
    //     //     .ok();
    //     return Some{FoundInfo{addr, 576_KHz}};
    // };

    // const auto addresses = std::views::iota(0, 128)
    //     | std::views::transform([](uint8_t i) { return i << 1; });


    // const auto founded_devices = addresses
    //     | std::views::transform(process_address)
    //     | std::views::filter([](auto opt) { return opt.is_some(); })
    //     | std::views::transform([](auto opt) { return opt.unwrap(); })
        // | std::ranges::to<std::vector>()
        ;
    
    // addresses.
    // founded_devices.to<std::vector<FoundInfo>>();   
    // founded_devices
    // const auto vi =
    //     std::views::for_each(std::views::ints(1, 10), [](int i) {
    //         return yield_from(std::views::repeat_n(i, i));
    //     })
    //   | std::ranges::to<std::vector>();
}