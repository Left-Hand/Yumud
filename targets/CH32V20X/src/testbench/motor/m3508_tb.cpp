#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/utils/zero.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/conn/can/hw_singleton.hpp"

#include "algebra/transforms/Basis.hpp"

#include "robots/vendor/dji/c620/c620.hpp"
#include "robots/vendor/dji/dr16/dr16.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

using namespace ymd::robots::dji;


#if 1
void m3508_main(){
    static constexpr size_t CB_FREQ = 200;

    auto led = hal::PC<14>();
    led.outpp(HIGH);

    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    auto & can = hal::can1;

    can.init({
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanNominalBitTimming(hal::CanBaudrate::_1M)
    });


    can.configure_filter(
        0_nth,
        hal::CanFifoIndex::_0, 
        hal::CanFilterConfig::accept_all()
    ).unwrap();
    
    auto & timer = hal::timer3;

    timer.init({
        .remap = hal::TimerRemap::_0,
        .count_freq = hal::NearestFreq(CB_FREQ), 
        .count_mode = hal::TimerCountMode::Up
    }).unwrap().dont_alter_to_pins();

    timer.register_nvic<hal::TimerIT::Update>(hal::NvicPriorityCode::highest(),  EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_callback([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            
            break;
        }
        default: break;
        }
    });

    auto write_can_frame = [](const hal::ClassicCanFrame & frame){
        can.try_write(frame).examine();
    };


    while(true){
        const auto now_secs = clock::seconds();

        const iq16 x1 = 3 * iq16(sin(2 * now_secs));
        // const iq16 x2 = 6 * iq16(cos(2 * now_secs));

        const auto current_code = c620::CurrentCode::from_amps_bounded(x1);

        const auto can_frame = c620::TxContext{
            .current_codes = {
                current_code, 
                Zero,
                Zero,
                Zero
            }
        }.to_can_frame(c620::LOWER_QUAD_CANID);

        write_can_frame(can_frame);

        led = BoolLevel::from((clock::millis().count() % 400) > 200);

    }
}

#endif