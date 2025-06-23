#include "src/testbench/tb.h"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "robots/cannet/slcan/slcan.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"

using namespace ymd;


[[maybe_unused]] static __attribute__((__noreturn__))
void slcan_main(){
    auto & OUT_UART = hal::uart1;
    OUT_UART.init({576000});

    DEBUGGER.retarget(&OUT_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    hal::can1.init({hal::Can::BaudRate::_1M});
    robots::asciican::AsciiCanPhy phy{hal::can1};
    robots::asciican::Slcan slcan{phy};
    auto list = rpc::make_list(
        "slcan",
        rpc::make_function("tx", [&](const StringView str) {
            const auto u_begin = clock::micros();
            const auto res = slcan.on_recv_string(str);
            const auto u_end = clock::micros();
            DEBUG_PRINTLN(res, u_end - u_begin);
            // DEBUG_PRINTLN(res.is_err());
        })
    );

    robots::ReplService repl_service = {
        &OUT_UART, &OUT_UART
    };


    while(true){
        // DEBUG_PRINTLN(clock::millis());
        repl_service.invoke(list);
    }
}