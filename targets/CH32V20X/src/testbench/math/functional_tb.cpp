#include <bitset>
#include <ranges>


#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"
#include "core/utils/Result.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "hal/bus/uart/uarthw.hpp"


using namespace ymd;


struct FoundInfo{
    uint8_t addr;
    uint max_bbaud;
};


#define UART hal::uart2

namespace ranges = std::ranges;

void functional_main(){
    DEBUGGER_INST.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000 
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.force_sync(EN);
    
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