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
#include "robots/vendor/jvci/jvci_framefactory.hpp"

#include "drivers/common_io/Key/Key.hpp"

using namespace ymd;
using namespace ymd::robots;

static constexpr uint8_t NUM_LEFT_MOTOR_AXIS_ID = 0x01;
static constexpr uint8_t NUM_RIGHT_MOTOR_AXIS_ID = 0x02;



void jvci_main(){

    auto & DBG_UART = hal::usart2;

    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking 
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.build_config()
        .set_eps(5)
        .set_splitter(",")
        .no_brackets(EN) 
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();

    auto led_pin = hal::PC<13>();
    led_pin.outpp();

    auto poll_led = [&]{
        const uint32_t now_millis = static_cast<uint32_t>(clock::millis().count());
        led_pin.write(BoolLevel::from(now_millis % 200 > 100));
    };


    auto & can = hal::can1;
    //初始化CAN外设
    can.init({
        //映射到TX:PA12 RX:PA11
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        //波特率为1M
        // .bit_timming = hal::CanNominalBitTimming(hal::CanBaudrate::_1M)
        .bit_timming = hal::CanNominalBitTimmingCoeffs{
            .prescale = 9,
            .swj = hal::CanTq::from_num(2),
            .bs1 = hal::CanTq::from_num(12),
            .bs2 = hal::CanTq::from_num(3)
        }
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
        const auto res = can.try_write(frame);
        if(res.is_err()) return;
        if(timeout_ms != 0ms) clock::delay(timeout_ms);
    };

    clock::delay(100ms);

    // [[maybe_unused]] auto & timer = hal::timer2;
    // //配置定时器
    // timer.init({
    //     .remap = hal::TIM2_REMAP_A0_A1_A2_A3,
    //     .count_freq = hal::NearestFreq(500), 
    //     .count_mode = hal::TimerCountMode::Up
    // })  .unwrap()
    //     .dont_alter_to_pins()
    // ;


    static constexpr jvci::CanRequestFrameFactory left_factory{
        .node_id = jvci::NodeId::try_from_u8(NUM_LEFT_MOTOR_AXIS_ID).unwrap()
    };

    static constexpr jvci::CanRequestFrameFactory right_factory{
        .node_id = jvci::NodeId::try_from_u8(NUM_RIGHT_MOTOR_AXIS_ID).unwrap()
    };


    write_can_frame(left_factory.set_control_mode(jvci::ControlMode::Speed), 2ms);
    write_can_frame(left_factory.enter_close_loop(), 2ms);
    write_can_frame(right_factory.set_control_mode(jvci::ControlMode::Speed), 2ms);
    write_can_frame(right_factory.enter_close_loop(), 2ms);
    
    while(true){
        [[maybe_unused]] const auto now_secs = clock::seconds();
        
        poll_led();
        
        #if 0
        const iq16 turns = iq16(math::sin(now_secs)) * 0.3_iq16;
        const auto left_position_code = jvci::PositionCode::from_turns(turns);
        const auto right_position_code = jvci::PositionCode::from_turns(-turns);

        write_can_frame(left_factory.set_absolute_position(left_position_code), 2ms);
        write_can_frame(right_factory.set_absolute_position(right_position_code), 2ms);
        #else
        // const iq16 left_x2 = iq16(math::sin(now_secs)) * 20.3_iq16;
        // const iq16 right_x2 = iq16(math::cos(now_secs)) * 20.3_iq16;

        const iq16 left_x2 = 32 * 17.3_iq16;
        const iq16 right_x2 = 32 * 22.3_iq16;


        const auto left_speed_code = jvci::SpeedCode::from_rpm_bounded(left_x2);
        const auto right_speed_code = jvci::SpeedCode::from_rpm_bounded(-right_x2);

        write_can_frame(left_factory.set_speed(left_speed_code), 2ms);
        write_can_frame(right_factory.set_speed(right_speed_code), 2ms);

        DEBUG_PRINTLN(left_speed_code.bits);
        #endif

        while(can.available()){
            handle_can_frame(can.try_read().unwrap());
        }

        // clock::delay(20ms);
    }
}

