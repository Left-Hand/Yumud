#include "../tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/timer/instance/timer_hw.hpp"

using namespace hal;
using Can = hal::Can;
void can_ring_main(){
    // scexpr size_t cb_freq = 200;

    auto & led = portC[14];
    led.outpp(HIGH);

    DEBUGGER_INST.init(576000, CommStrategy::Blocking);
    auto & can = can1;
    can.init(1_MHz, Can::Mode::Internal);

    can[0].mask(
        CanID16{0x200, Can::RemoteType::Any}, CanID16::IGNORE_LOW(7, Can::RemoteType::Any),
        CanID16{0x000, Can::RemoteType::Any}, CanID16::IGNORE_LOW(7, Can::RemoteType::Any));

    while(true){

        constexpr auto ids = std::to_array<uint16_t>({0x200, 0x201, 0x401, 0x402, 0x120});

        for(auto i : ids){
            CanMsg tx_msg = {i, std::make_tuple(0x12345678)};
            can.write(tx_msg);
        }

        {
            CanMsg tx_msg = {0x201};
            can.write(tx_msg);
            // DEBUG_PRINTLN(tx_msg);
        }

        delay(100);

        if(can.available()){
            while(can.available()){
                auto rx_msg = can.read();
                DEBUG_PRINTLN(rx_msg);
            }
        }else{
            DEBUG_PRINTLN('N');
        }
    }
}