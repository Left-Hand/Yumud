#include "src/testbench/tb.h"

#include "primitive/arithmetic/percentage.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/math/realmath.hpp"
#include "core/async/timer.hpp"
#include "core/utils/zero.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/conn/can/hw_singleton.hpp"

#include "robots/vendor/jvci/jvci_primitive.hpp"

#include "drivers/common_io/Key/Key.hpp"

using namespace ymd;
using namespace ymd::robots;

static constexpr uint8_t LEFT_MOTOR_AXIS_ID = 0x01;
static constexpr uint8_t RIGHT_MOTOR_AXIS_ID = 0x02;

void jvci_main(){
    auto & DBG_UART = hal::usart2;
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
        .tx_strategy = CommStrategy::Blocking 
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.no_fieldname(EN);

    auto & can = hal::can1;
    //初始化CAN外设
    can.init({
        //映射到TX:PA12 RX:PA11
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        //波特率为1M
        .bit_timming = hal::CanNominalBitTimming(hal::CanBaudrate::_1M), 
    });
    
    //配置can过滤器为接收标准数据帧（滤除拓展和远程帧）
    can.configure_filter(
        0_nth, 
        hal::CanFifoIndex::_0,
        hal::CanFilterConfig::from_std_pairs(
            hal::CAN_FILTER_PAIR_STD_DATA_FRAME_ONLY,
            hal::CanStdIdMaskPair::reject_all()
        )
    ).unwrap();


    [[maybe_unused]] auto handle_reponse = [&](
        const jvci::NodeId node_id, 
        const hal::ClassicCanPayload payload
    ){


    };

    [[maybe_unused]] auto handle_can_frame = [&](const hal::ClassicCanFrame & frame){
        if(frame.is_extended()) PANIC{"ext can frame received!!"};
        if(frame.length() != 8) PANIC{"can frame length is not 8", frame.length()};

        const auto node_id = ({
            const auto may_node_id = jvci::exact_response_canid(frame.identifier().to_stdid());
            if(may_node_id.is_none()) return;
            may_node_id.unwrap();
        });

        handle_reponse(node_id, frame.payload().clone());
    };

    [[maybe_unused]] auto write_can_frame = [](const hal::ClassicCanFrame & frame, const Milliseconds timeout_ms = 0ms){
        can.try_write(frame).examine();
        if(timeout_ms != 0ms) clock::delay(timeout_ms);
    };

    clock::delay(100ms);

    [[maybe_unused]] auto & timer = hal::timer2;
    //配置定时器
    timer.init({
        .remap = hal::TIM2_REMAP_A0_A1_A2_A3,
        .count_freq = hal::NearestFreq(500), 
        .count_mode = hal::TimerCountMode::Up
    })  .unwrap()
        .dont_alter_to_pins()
    ;


    while(true){
        // const auto now_secs = clock::seconds();


    }
}

