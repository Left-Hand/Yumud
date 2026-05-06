#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/async/timer.hpp"
#include "core/clock/time.hpp"

#include "primitive/colors/color/color.hpp"
#include "algebra/vectors/vec2.hpp"


#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/conn/can/hw_singleton.hpp"

#include "robots/vendor/zdt/zdt_frame_factory.hpp"

#ifdef USART1_PRESENT
using namespace ymd;

using namespace ymd::robots;

#define DBG_UART hal::usart2
#define COMM_UART hal::usart1
#define COMM_CAN hal::can1

#define PHY_SEL_CAN 0
#define PHY_SEL_UART 1

// #define PHY_SEL PHY_SEL_UART
#define PHY_SEL PHY_SEL_CAN

void zdt_main(){
    DBG_UART.init({
        .remap = hal::UartRemap::_0,
        .baudrate = hal::NearestFreq(576_KHz),
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.build_config()
        .set_eps(4)
        .set_splitter(",")
        .no_brackets(EN)
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();

    zdtmotor::ZdtFrameFactory factory1 = zdtmotor::ZdtFrameFactory::with_node_id(1);
    zdtmotor::ZdtFrameFactory factory2 = zdtmotor::ZdtFrameFactory::with_node_id(2);

    #if PHY_SEL == PHY_SEL_UART
    COMM_UART.init({
        .remap = hal::UartRemap::_0,
        .baudrate = hal::NearestFreq(576_KHz),
    });

    auto write_packet = [&](const zdtmotor::FlatPacket & packet){
        auto bytes = packet.transmittable_bytes();
        const size_t act_len = COMM_UART.try_write_bytes(bytes);
        if(bytes.size() != act_len){

        }
    };
    #else
    COMM_CAN.init({
        .remap = hal::CanRemap::_0,
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanNominalBitTimming(hal::CanBaudrate::_1M)
    });

    COMM_CAN.enable_hw_retransmit(DISEN);
    auto write_packet = [&](const zdtmotor::FlatPacket & packet){
        auto && iter = packet.to_canframe_iter();
        while(iter.has_next()){
            auto can_frame = iter.next();
            hal::can1.try_write(can_frame).examine();
        }
    };
    #endif
    
    clock::delay(10ms);
    write_packet(factory1.activate(EN));
    write_packet(factory2.activate(EN));
    clock::delay(10ms);


    while(true){
        #if PHY_SEL == PHY_SEL_UART
        if(COMM_UART.available()){
            std::vector<uint8_t> recv;
            while(COMM_UART.available()){
                uint8_t chr;
                size_t len = COMM_UART.try_read_byte(chr);
                if(len) recv.push_back(chr);
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
            DEBUG_PRINTLN("rx", COMM_CAN.try_read().unwrap());
        }

        // DEBUG_PRINTLN(COMM_CAN.pending());

        // clock::delay(200ms);
        // motor.activate();
        const auto d1 = static_cast<iq16>(math::sin(clock::seconds()*0.7_r));
        const auto d2 = static_cast<iq16>(math::sin(clock::seconds()*0.2_r));
        write_packet(factory1.set_angle(
            Angular<iq16>::from_turns(d1), 
            0
        ));

        clock::delay(5ms);
        write_packet(factory2.set_angle(
            Angular<iq16>::from_turns(d2), 
            0
        ));
        clock::delay(5ms);
        DEBUG_PRINTLN(d1, d2);
        // DEBUG_PRINTLN(clock::millis());
        #endif

    }

}
#endif