#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "core/math/realmath.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;

void m2006_main(){
    // scexpr size_t cb_freq = 200;

    auto & led = portC[14];
    led.outpp(HIGH);

    // DEBUGGER_INST.init(576000, CommStrategy::Blocking);
    auto & can = can1;
    can.init({CanBaudrate::_1M});
    can[0].mask(
        {   
            .id = CanStdIdMask{0x201, CanRemoteSpec::Data}, 
            .mask = CanStdIdMask{0xffff, CanRemoteSpec::Remote}
        }
    );
    while(true){
        auto s = real_t(0.07) * sin(4 * clock::time());
        auto c = real_t(0.07) * cos(4 * clock::time());
        // real_t s = real_t(0.07);
        // real_t c = real_t(0.07);
        int16_t d = int16_t(s * 32768 * real_t(0.2));
        int16_t d2 = int16_t(c * 32768 * real_t(0.2));

        struct Payload{
            int16_t d;
            int16_t d2;
        };

        CanMsg msg = CanMsg::from_bytes(
            CanStdId(0x200), 
            std::bit_cast<std::array<uint8_t, 4>>(
                Payload{BSWAP_16(d), BSWAP_16(d2)}
            )
        );
        DEBUG_PRINTLN(can1.read());
        can1.write(msg).examine();
        clock::delay(10ms);
    }
}