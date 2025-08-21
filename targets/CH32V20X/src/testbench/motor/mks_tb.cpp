#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "robots/vendor/mks/mks_stepper.hpp"
#include "core/string/string_view.hpp"
#include "types/vectors/vector2.hpp"

#include "core/sync/timer.hpp"
#include "types/colors/color/color.hpp"


#ifdef ENABLE_UART1
using namespace ymd;
using namespace ymd::hal;
using namespace ymd::robots;
using robots::mksmotor::MksStepper;

#define DBG_UART hal::uart2
#define COMM_UART hal::uart1
#define COMM_CAN hal::can1

#define PHY_SEL_CAN 0
#define PHY_SEL_UART 1

// #define PHY_SEL PHY_SEL_UART
#define PHY_SEL PHY_SEL_CAN

void mks_stepper_main(){
    // slcan_test();
    DBG_UART.init({576000});

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    #if PHY_SEL == PHY_SEL_UART
    COMM_UART.init({921600});
    MksStepper motor{{.nodeid = {1}}, &COMM_UART};
    #else
    COMM_CAN.init({
        .baudrate = CanBaudrate::_1M, 
        .mode = CanMode::Normal
    });

    COMM_CAN.enable_hw_retransmit(DISEN);
    MksStepper motor1{{.nodeid = {1}}, &COMM_CAN};
    MksStepper motor2{{.nodeid = {2}}, &COMM_CAN};
    #endif
    
    clock::delay(10ms);
    motor1.activate().unwrap();
    motor2.activate().unwrap();
    clock::delay(10ms);


    while(true){
        #if PHY_SEL == PHY_SEL_UART
        if(COMM_UART.available()){
            std::vector<uint8_t> recv;
            while(COMM_UART.available()){
                char chr;
                COMM_UART.read1(chr);
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
        const auto d1 = sin(clock::time()*0.7_r);
        const auto d2 = tpzpu(clock::time()*0.2_r);
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