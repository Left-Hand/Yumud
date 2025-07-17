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
    can.init({hal::CanBaudrate::_1M, Can::Mode::Loopback});
    // can.init({hal::CanBaudrate::_1M, Can::Mode::Normal});

    can[0].mask(
        {
            .id = CanStdIdMask{0x200, CanRemoteSpec::Any}, 
            .mask = CanStdIdMask::from_ignore_low(7, CanRemoteSpec::Any)
        },{
            .id = CanStdIdMask{0x000, CanRemoteSpec::Any}, 
            .mask = CanStdIdMask::from_ignore_low(7, CanRemoteSpec::Any)
        }
    );

    auto write_msg = [&](const hal::CanMsg & msg){
        can.write(msg);
        DEBUG_PRINTLN("tx", msg);
    };

    while(true){
        write_msg(CanMsg::from_list(CanStdId(0x10), {0, 1, 3}));
        clock::delay(2ms);
        write_msg(CanMsg::from_bytes(CanStdId(0x20), std::bit_cast<std::array<uint8_t, 4>>(0x12345678)));
        clock::delay(2ms);
        write_msg(CanMsg::from_bytes(CanStdId(0x30), std::bit_cast<std::array<uint8_t, 4>>(0x12345678)));
        clock::delay(2ms);
        write_msg(CanMsg::from_bytes(CanExtId(0x40), std::bit_cast<std::array<uint8_t, 4>>(0x12345678)));

        clock::delay(100ms);

        if(can.available()){
            DEBUG_PRINTLN(can.available());
            while(can.available()){
                auto rx_msg = can.read();
                DEBUG_PRINTLN("rx", rx_msg);
            }
        }else{
            DEBUG_PRINTLN("no msg received");
        }
    }

}