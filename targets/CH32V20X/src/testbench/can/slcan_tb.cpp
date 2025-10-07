#include "src/testbench/tb.h"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "robots/cannet/slcan/slcan.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"

using namespace ymd;



void slcan_main(){
    auto & DBG_UART = hal::uart2;
    DBG_UART.init({576000});

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    auto & can = hal::can1;
    can.init({
        .coeffs = hal::CanBaudrate(hal::CanBaudrate::_1M).to_coeffs() 
    });
    robots::asciican::AsciiCanPhy phy{can};
    robots::slcan::Slcan slcan{phy};
    auto list = rpc::make_list(
        "slcan",
        rpc::make_function("tx", [&](const StringView str) {
            const auto u_begin = clock::micros();
            const auto res = slcan.on_recv_str(str);
            const auto u_end = clock::micros();
            DEBUG_PRINTLN(res, u_end - u_begin);
            // DEBUG_PRINTLN(res.is_err());
        })
    );

    robots::ReplServer repl_server{
        &DBG_UART, &DBG_UART
    };


    while(true){
        // DEBUG_PRINTLN(clock::millis());
        repl_server.invoke(list);
    }
}