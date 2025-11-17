#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "core/math/realmath.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

void m2006_main(){
    // static constexpr size_t cb_freq = 200;

    auto led = hal::PC<14>();
    led.outpp(HIGH);

    // DEBUGGER_INST.init(576000, CommStrategy::Blocking);
    auto & can = hal::can1;
    can.init({
        .remap = CAN1_REMAP,
        .mode = hal::CanMode::Normal,
        .timming_coeffs = hal::CanBaudrate(hal::CanBaudrate::_1M).to_coeffs(), 
    });

    can.filters<0>().apply(
        hal::CanFilterConfig::from_accept_all()
    );

    while(true){
        const auto ctime = clock::time();
        const auto [s, c] = sincospu(0.7_r * ctime);
        int16_t d = int16_t(32768 * real_t(0.0014) * s);
        int16_t d2 = int16_t(32768 * real_t(0.0014) * c);

        struct Payload{
            int16_t d;
            int16_t d2;
        };

        hal::CanClassicMsg msg = hal::CanClassicMsg::from_bytes(
            hal::CanStdId(0x200), 
            std::bit_cast<std::array<uint8_t, 4>>(
                Payload{BSWAP_16(d), BSWAP_16(d2)}
            )
        );
        DEBUG_PRINTLN(can.read());
        can.write(msg).examine();
        clock::delay(10ms);
    }
}