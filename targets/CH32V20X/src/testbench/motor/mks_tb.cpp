#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/async/timer.hpp"

#include "algebra/vectors/vec2.hpp"
#include "primitive/colors/color/color.hpp"


#include "robots/vendor/mks/mks_stepper.hpp"


#ifdef UART1_PRESENT
using namespace ymd;

using namespace ymd::robots;
using robots::mksmotor::MksStepper;

#define DBG_UART hal::usart2
#define COMM_UART hal::usart1
#define COMM_CAN hal::can1

#define PHY_SEL_CAN 0
#define PHY_SEL_UART 1

// #define PHY_SEL PHY_SEL_UART
#define PHY_SEL PHY_SEL_CAN

void mks_stepper_main(){
    // slcan_test();
    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz), 
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    #if PHY_SEL == PHY_SEL_UART
    COMM_UART.init({921600});
    MksStepper motor{{.nodeid = {1}}, &COMM_UART};
    #else
    COMM_CAN.init({
        .remap = hal::CanRemap::_0,
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanBaudrate(hal::CanBaudrate::_1M)
    });

    COMM_CAN.enable_hw_retransmit(DISEN);
    MksStepper motor1{{.nodeid = {1}}, &COMM_CAN};
    MksStepper motor2{{.nodeid = {2}}, &COMM_CAN};
    #endif
    
    clock::delay(10ms);
    motor1.activate(EN).unwrap();
    motor2.activate(EN).unwrap();
    clock::delay(10ms);


    while(true){
        #if PHY_SEL == PHY_SEL_UART
        if(COMM_UART.available()){
            std::vector<uint8_t> recv;
            while(COMM_UART.available()){
                char chr;
                COMM_UART.read_char(chr);
                recv.push_back(chr);
            }

            DEBUG_PRINTLN(
                "ret", 
                std::hex, 
                std::noshowbase, 
                recv
            );
        }
        #else
        if(COMM_CAN.available()){
            DEBUG_PRINTLN("rx", COMM_CAN.read());
        }

        // DEBUG_PRINTLN(COMM_CAN.pending());

        // clock::delay(200ms);
        // motor.activate();
        const auto d1 = math::sin(clock::seconds()*0.7_r);
        const auto d2 = math::sin(clock::seconds()*0.2_r);
        // motor1.set_position({.position = d1, .speed = 0}).unwrap();
        clock::delay(5ms);
        motor2.set_position({.position = 0, .speed = 0}).unwrap();
        // motor2.set_position({.position = d2, .speed = 0}).unwrap();
        clock::delay(5ms);
        DEBUG_PRINTLN(d1, d2);
        // DEBUG_PRINTLN(clock::millis());
        #endif

    }

}
#endif