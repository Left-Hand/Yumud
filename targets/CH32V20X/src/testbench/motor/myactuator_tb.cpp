#include "src/testbench/tb.h"

#include "primitive/arithmetic/percentage.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/utils/bytes/bytes_provider.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "core/utils/bytes/bytes_exacter.hpp"
#include "core/utils/enum/strong_type_gradation.hpp"
#include "core/math/float/fp32.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "robots/vendor/myactuator/myactuator_msgs.hpp"


using namespace ymd;

using namespace robots;

template<typename T>
__always_inline constexpr auto make_bytes(T && msg){

    std::array<uint8_t, 7> bytes;
    msg.fill_bytes(bytes);
    return bytes;
}

__no_inline auto make_bytes1(){
    return make_bytes(
        myactuator::req_msgs::SetTorquePosition{
            .rated_current_ratio = Percentage<uint8_t>::from_percents_unchecked(30),
            .max_speed = myactuator::SpeedLimitCode_u16{2},
            .position = myactuator::PositionCode_i32{0x7fff}
        }
    );
}

__no_inline auto make_bytes2(){
    return make_bytes(
        myactuator::req_msgs::SetTorquePosition{
            .rated_current_ratio = Percentage<uint8_t>::from_percents_unchecked(3),
            .max_speed = myactuator::SpeedLimitCode_u16{2},
            .position = myactuator::PositionCode_i32{0x7fff}
        }
    );
}
void myactuator_main(){
    auto & DBG_UART = DEBUGGER_INST;
    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz), 
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    DEBUG_PRINTLN(make_bytes1());
    DEBUG_PRINTLN(make_bytes2());
    auto & os = DEBUGGER;
    os.field("enabled")(make_bytes2());
    PANIC{};
    while(true);
}

