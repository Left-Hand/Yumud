#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;
using namespace ymd::hal;

using Can = hal::Can;
void can_ring_main(){
    DEBUGGER_INST.init({576000, CommStrategy::Blocking});
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);
    
    auto & led = portC[14];
    led.outpp(HIGH);

    auto & can = can1;
    // can.init({hal::CanBaudrate::_1M, Can::Mode::Internal});
    // can.init({hal::CanBaudrate::_1M, Can::Mode::Loopback});
    can.init({hal::CanBaudrate::_1M, Can::Mode::Normal});

    can[0].mask(
        CanID16{0x200, CanRemoteSpec::Any}, CanID16::IGNORE_LOW(7, CanRemoteSpec::Any),
        CanID16{0x000, CanRemoteSpec::Any}, CanID16::IGNORE_LOW(7, CanRemoteSpec::Any));

    auto write_msg = [&](const hal::CanMsg & msg){
        can.write(msg);
        DEBUG_PRINTLN("tx", msg);
    };

    while(true){

        // constexpr auto ids = std::to_array<uint16_t>({0x200, 0x201, 0x401, 0x402, 0x120});

        // for(auto i : ids){
        //     CanMsg tx_msg = CanMsg::from_tuple(CanStdId(i), std::make_tuple(0x12345678));
        //     can.write(tx_msg);
        //     clock::delay(2ms);
        // }
        // can.write(CanMsg::from_tuple(CanStdId(0x10), {0, 1}));
        write_msg(CanMsg::from_list(CanStdId(0x10), {0, 1, 3}));
        clock::delay(2ms);
        write_msg(CanMsg::from_tuple(CanStdId(0x20), std::make_tuple(0x12345678)));
        clock::delay(2ms);
        write_msg(CanMsg::from_tuple(CanStdId(0x30), std::make_tuple(0x12345678)));
        clock::delay(2ms);
        write_msg(CanMsg::from_tuple(CanExtId(0x40), std::make_tuple(0x12345678)));
        // {
        //     CanMsg tx_msg = CanMsg::from_remote(CanStdId(0x201));
        //     can.write(tx_msg);
        //     // DEBUG_PRINTLN(tx_msg);
        // }

        clock::delay(100ms);

        if(can.available()){
            DEBUG_PRINTLN(can.available());
            while(can.available()){
                auto rx_msg = can.read();
                DEBUG_PRINTLN("rx", rx_msg);
            }
        }else{
            DEBUG_PRINTLN('N');
        }
    }

}