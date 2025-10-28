#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "robots/vendor/zdt/zdt_stepper.hpp"
#include "core/string/string_view.hpp"
#include "types/vectors/vector2.hpp"

#include "core/sync/timer.hpp"
#include "types/colors/color/color.hpp"


#ifdef ENABLE_UART1
using namespace ymd;

using namespace ymd::robots;
using robots::zdtmotor::ZdtStepper;

#define DBG_UART hal::uart2
#define COMM_UART hal::uart1
#define COMM_CAN hal::can1

#define PHY_SEL_CAN 0
#define PHY_SEL_UART 1

// #define PHY_SEL PHY_SEL_UART
#define PHY_SEL PHY_SEL_CAN

void zdt_main(){
    // slcan_test();
    DBG_UART.init({576000});

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    #if PHY_SEL == PHY_SEL_UART
    COMM_UART.init({921600});
    ZdtStepper motor{{.nodeid = {1}}, &COMM_UART};
    #else
    COMM_CAN.init({
        .remap = 0,
        .mode = hal::CanMode::Normal,
        .timming_coeffs = hal::CanBaudrate(hal::CanBaudrate::_1M).to_coeffs()
    });

    COMM_CAN.enable_hw_retransmit(DISEN);
    ZdtStepper motor1{{.node_id = {1}}, &COMM_CAN};
    ZdtStepper motor2{{.node_id = {2}}, &COMM_CAN};
    #endif
    
    clock::delay(10ms);
    motor1.activate(EN).examine();
    motor2.activate(EN).examine();
    clock::delay(10ms);
    // motor.set_subdivides(256);
    // motor.trig_homming(ZdtStepper::HommingMode::LapsCollision);
    // motor.trig_homming(ZdtStepper::HommingMode::LapsEndstop);
    // motor.query_homming_paraments();

    // auto list = rpc::make_list(
    //     "list",

    // );

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
        motor1.set_position({
            .position = Angle<real_t>::from_turns(d1), 
            .speed = 0
        }).examine();

        clock::delay(5ms);
        motor2.set_position({
            .position = Angle<real_t>::from_turns(d2), 
            .speed = 0
        }).examine();
        clock::delay(5ms);
        DEBUG_PRINTLN(d1, d2);
        // DEBUG_PRINTLN(clock::millis());
        #endif

    }

}
#endif