#include "src/testbench/tb.h"

#include "primitive/arithmetic/percentage.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/math/realmath.hpp"
#include "core/async/timer.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/can/can.hpp"

#include "robots/vendor/bmkj/m1502e_highlayer.hpp"

#include "dsp/controller/adrc/linear/ltd2o.hpp"
#include "dsp/controller/adrc/linear/leso2o.hpp"
#include "dsp/controller/adrc/nonlinear/nltd2o.hpp"

#include "drivers/CommonIO/Key/Key.hpp"
#include "drivers/Proximeter/ALX_AOA/alx_aoa_prelude.hpp"
#include "robots/vendor/steadywin/can_simple/steadywin_can_simple_msgs.hpp"

using namespace ymd;
using namespace robots::steadywin::can_simple;


void steadywin_main(){
    auto & DBG_UART = hal::usart2;
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
        .tx_strategy = CommStrategy::Blocking 
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    auto & can = hal::can1;
    //初始化CAN外设
    can.init({
        //映射到TX:PA12 RX:PA11
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        //波特率为1M
        .bit_timming = hal::CanBaudrate(hal::CanBaudrate::_1M), 
    });

    [[maybe_unused]] static constexpr auto STD_DATA_FRAME_ONLY_FILTER_PAIR = 
        hal::CanStdIdMaskPair::from_str("xxxxxxxxxxx", hal::CanRtrSpecfier::DataOnly).unwrap();
    
    const auto FILTER_CONFIG = hal::CanFilterConfig::accept_all();
    // PANIC{FILTER_CONFIG};
    //配置can过滤器为接收标准数据帧（滤除拓展和远程帧）
    can.filters<0>().apply(
        // hal::CanFilterConfig::from_pairs(
        //     STD_DATA_FRAME_ONLY_FILTER_PAIR,
        //     hal::CanStdIdMaskPair::reject_all()
        // )
        // hal::CanFilterConfig::accept_all()
        FILTER_CONFIG
    );


    while(true){
        const auto frame = serialize_msg_to_can_frame(
            AxisId::from_bits(0x01),
            req_msgs::SetAxisState{
                .axis_state = AxisState::MotorCalibration
            }
        );

        // const auto frame = hal::BxCanFrame::from_parts(
        //     hal::CanStdId::from_u11(0x111),
        //     hal::BxCanPayload::from_list({0x01, 0x02, 0x03, 0x04})
        // );
        can.try_write(frame).examine();
        // static constexpr auto frame = serialize_msg_to_can_frame(
        //     AxisId::from_bits(0), 
        //     req_msgs::SetInputPosition{
        //         .input_position = 2.2,
        //         .vel_ff = 0,
        //         .torque_ff = 0,
        //     }
        // );
        DEBUG_PRINTLN_IDLE(
            can.available(), 
            hal::PA<12>().read().to_bool(),
            hal::PA<11>().read().to_bool(),
            can.last_error(),
            clock::millis().count(),
            can.get_tx_errcnt(),
            can.get_rx_errcnt(),
            frame
        );
        // if(can.available()){
        //     DEBUG_PRINTLN_IDLE(can.read());
        // }
        clock::delay(20ms);
    }
}

