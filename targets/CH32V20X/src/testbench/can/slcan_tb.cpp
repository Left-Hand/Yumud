#include "src/testbench/tb.h"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "core/clock/time_stamp.hpp"

#include "middlewares/protocols/cannet/slcan/slcan.hpp"
#include "middlewares/rpc/rpc.hpp"
#include "middlewares/rpc/repl_server.hpp"

using namespace ymd;



void slcan_main(){
    auto & DBG_UART = DEBUGGER_INST;
    DBG_UART.init({
        hal::USART2_REMAP_PA2_PA3,
        hal::NearestFreq(576000),
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    auto & can = hal::can1;
    can.init({
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanBaudrate(hal::CanBaudrate::_1M)
    });

    // asciican::AsciiCanPhy phy{can};
    [[maybe_unused]] robots::slcan::SlcanParser slcan_parser;
    // auto list = rpc::make_list(
    //     "slcan",
    //     rpc::make_function("tx", [&](const StringView str) {
    //         const auto u_begin = clock::micros();
    //         const auto res = slcan.on_recv_str(str);
    //         const auto u_end = clock::micros();
    //         DEBUG_PRINTLN(res, u_end - u_begin);
    //         // DEBUG_PRINTLN(res.is_err());
    //     })
    // );

    [[maybe_unused]] robots::ReplServer repl_server{
        &DBG_UART, &DBG_UART
    };

    std::array lines = {
        // Initialization and configuration commands
        "C",                    // Close channel
        "O",                    // Open channel
        "S3",                   // Set bitrate to 500 kbps
        "S9",                   // Set bitrate to 500 kbps
        "C",                    // Close channel again
        "O",                    // Reopen channel
        
        // Standard frame transmission (11-bit ID)
        "t1230",                // Transmit standard frame with ID 0x123, no data
        "t1231AA",              // Transmit standard frame with ID 0x123, 1 byte data
        "t1234AA55AA55",    // Transmit standard frame with ID 0x123, 4 bytes data
        "t1FFFFFFFF0", 
        "T1FFFFFFFFF0", //-> Ok(...)           # 扩展帧正常(0x1FFFFFFF)
        "T2000000000", //-> Err(IdOutOfRange)  # 扩展帧ID超限(>0x1FFFFFFF)
        // Extended frame transmission (29-bit ID)
        "T12345670",            // Transmit extended frame with ID 0x1234567, no data
        
        // Standard frame reception filter setup
        "r1230",                // Add filter for standard ID 0x123
        "r4561",                // Add filter for standard ID 0x456 with mask 0x1
        
        // Extended frame reception filter setup
        "R12345670",            // Add filter for extended ID 0x1234567
        "R1234567F2",           // Add filter for extended ID 0x1234567 with mask 0xF2
        
        // Status and control commands
        "F",                    // Status flags query
        "V",                    // Hardware version query
        "v",                    // Firmware version query
        
        // Auto-polling mode
        "Z0",                   // Disable auto-polling
        "Z1",                   // Enable auto-polling
        
        // Timestamp handling
        "Z2",                   // Enable auto-polling with timestamp
        
        // Error conditions and edge cases
        "tXYZ0",                // Invalid hex characters in ID
        "T1234567G0",           // Invalid character in extended ID
        
        // Close at end
        "C\r\r\r\r\r"                     // Close channel
    };

    // DEBUG_PRINTLN(slcan.handle_line("C"));
    const auto used = measure_total_elapsed_us([&]{
        for(const auto line_cstr: lines){
            (void)slcan_parser.process_line(StringView::from_cstr(line_cstr));
        }
    });


    for(const auto line_cstr: lines){
        DEBUG_PRINTS(line_cstr,"->",slcan_parser.process_line(StringView::from_cstr(line_cstr)));
    }

    DEBUG_PRINTS("parse lines totally used:", used);

    while(true){
        // DEBUG_PRINTLN(clock::millis());
        // repl_server.invoke(list);

    }
}