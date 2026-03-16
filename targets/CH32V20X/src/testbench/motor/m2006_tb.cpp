#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/conn/can/hw_singleton.hpp"
#include "hal/timer/hw_singleton.hpp"

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
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanNominalBitTimming(hal::CanBaudrate::_1M), 
    });

    can.configure_filter(
        0_nth, 
        hal::CanFifoIndex::_0,
        hal::CanFilterConfig::accept_all()
    ).unwrap();

    while(true){
        const auto now_secs = clock::seconds();
        const auto [s, c] = math::sincospu(0.7_r * now_secs);
        int16_t d = int16_t(32768 * iq16(0.0014) * s);
        int16_t d2 = int16_t(32768 * iq16(0.0014) * c);

        struct Payload{
            int16_t d;
            int16_t d2;

            constexpr std::array<uint8_t, 4> to_bytes() const{
                return std::bit_cast<std::array<uint8_t, 4>>(*this);
            }
        };

        hal::ClassicCanFrame msg = hal::ClassicCanFrame::from_parts(
            hal::CanStdId::from_bits(0x200), 
            hal::ClassicCanPayload::from_bytes(
                Payload{__bswap16(d), __bswap16(d2)}.to_bytes()
            )
        );
        DEBUG_PRINTLN(can.try_read().unwrap());
        can.try_write(msg).examine();
        clock::delay(10ms);
    }
}