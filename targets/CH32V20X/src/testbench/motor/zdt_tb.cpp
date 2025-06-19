#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "robots/vendor/zdt/zdt_stepper.hpp"


using namespace ymd;
using namespace ymd::hal;
using namespace ymd::robots;

#define DBG_UART hal::uart2
#define COMM_UART hal::uart1
#define COMM_CAN hal::can1

#define PHY_SEL_CAN 0
#define PHY_SEL_UART 1

#define PHY_SEL PHY_SEL_UART


struct PolarRobotSolver{
    // struct Solution{
    //     real_t 
    // };

    struct Gesture{
        real_t x_meters;
        real_t y_meters;
    };
};


void zdt_main(){
    DBG_UART.init({576000});

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    #if PHY_SEL == PHY_SEL_UART
    COMM_UART.init({921600});
    ZdtStepper motor{{.nodeid = {1}}, &COMM_UART};
    #else
    COMM_CAN.init({CanBaudrate::_1M});
    ZdtStepper motor{{.nodeid = {1}}, &COMM_CAN};
    #endif
    
    clock::delay(10ms);
    motor.enable();
    clock::delay(10ms);
    // motor.set_subdivides(256);
    // motor.trig_homming(ZdtStepper::HommingMode::LapsCollision);
    motor.trig_homming(ZdtStepper::HommingMode::LapsEndstop);
    // motor.query_homming_paraments();

    while(true){
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
    }

    clock::delay(10ms);

    // motor.trigger_cali();
    while(true){
        const auto t = clock::time();
        clock::delay(10ms);
        // motor.enable();
        const auto targ_pos = 0.1_r * sin(t);
        motor.set_target_position(targ_pos);

        // motor.set_target_position(sin(clock::time()));    
        clock::delay(10ms);
        // DEBUG_PRINTLN(clock::millis());

            
        #if PHY_SEL == PHY_SEL_UART
        if(COMM_UART.available()){
            std::vector<uint8_t> recv;
            while(COMM_UART.available()){
                char chr;
                COMM_UART.read1(chr);
                recv.push_back(chr);
            }

            DEBUG_PRINTLN(std::hex, std::noshowbase, recv);
        }
        #else
        if(COMM_CAN.available()) DEBUG_PRINTLN(COMM_CAN.read());
        #endif
        

        // DEBUG_PRINTLN(COMM_CAN.pending(), COMM_CAN.getRxErrCnt(), COMM_CAN.getTxErrCnt());
    }
}