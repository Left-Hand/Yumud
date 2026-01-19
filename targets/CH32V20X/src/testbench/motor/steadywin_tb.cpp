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
#include "robots/vendor/steadywin/can_simple/steadywin_can_simple_factory.hpp"

#include "core/container/heapless_binaryheap.hpp"

using namespace ymd;
using namespace robots::steadywin;
using namespace robots::steadywin::can_simple;


struct EncoderFeedback{
    using Self = EncoderFeedback;
    Angular<iq14> multilap_angle;
    Angular<uq32> lap_angle;

    static constexpr Self zero(){
        return Self{
            .multilap_angle = Zero,
            .lap_angle = Zero,
        };
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self){ 
        return os << os.field("multilap_angle")(self.multilap_angle.to_turns()) << os.splitter()
            << os.field("lap_angle")(self.lap_angle.to_turns());
    }
};


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
    DEBUGGER.no_brackets(EN);
    DEBUGGER.no_fieldname(EN);

    auto & can = hal::can1;
    //初始化CAN外设
    can.init({
        //映射到TX:PA12 RX:PA11
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        //波特率为1M
        .bit_timming = hal::CanBaudrate(hal::CanBaudrate::_1M), 
    });
    
    //配置can过滤器为接收标准数据帧（滤除拓展和远程帧）
    can.filters<0>().apply(
        hal::CanFilterConfig::from_pairs(
            hal::CAN_FILTER_PAIR_STD_DATA_FRAME_ONLY,
            hal::CanStdIdMaskPair::reject_all()
        )
        // hal::CanFilterConfig::accept_all()
        // FILTER_CONFIG
    );

    can.enable_hw_retransmit(EN);
    std::array<EncoderFeedback, 2> encoder_feedbacks = {Zero, Zero};

    auto axis_id_to_idx = [](const AxisId axis_id) -> size_t {
        switch(axis_id.to_bits()){
            case 1: return 0;
            case 2: return 1;
        }
        PANIC{};
    };
    auto store_encoder_feedback = [&](const size_t idx, const resp_msgs::GetEncoderCount & msg){
        encoder_feedbacks.at(idx) = EncoderFeedback{
            .multilap_angle = msg.multilap_angle,
            .lap_angle = msg.lap_angle
        };
    };
    auto accept_either_msg = [&]<typename T>(const AxisId axis_id, const Result<T, DeMsgError> & either_msg){
        // DEBUG_PRINTLN(axis_id, either_msg);
        if constexpr (std::is_same_v<T, resp_msgs::GetEncoderEstimates>){
            // DEBUGGER.no_fieldname(EN);
            if(not either_msg.is_ok()) return;
            const resp_msgs::GetEncoderEstimates msg = either_msg.unwrap();
            // DEBUG_PRINTLN(msg.position, msg.velocity);
        }

        if constexpr (std::is_same_v<T, resp_msgs::HeartbeatV513>){
            // DEBUGGER.no_fieldname(EN);
            if(not either_msg.is_ok()) return;
            const resp_msgs::HeartbeatV513 msg = either_msg.unwrap();
            // DEBUG_PRINTLN(msg);
        }

        if constexpr (std::is_same_v<T, resp_msgs::GetEncoderCount>){
            // DEBUGGER.no_fieldname(EN);
            if(not either_msg.is_ok()) return;
            const resp_msgs::GetEncoderCount msg = either_msg.unwrap();
            // DEBUG_PRINTLN(msg.lap_angle.to_turns(), msg.multilap_angle.to_turns());
            store_encoder_feedback(axis_id_to_idx(axis_id), msg);
        }
    };

    [[maybe_unused]] auto parse_can_frame = [&](const hal::BxCanFrame & frame){
        if(frame.is_extended()) PANIC{};
        if(frame.length() != 8) PANIC{frame.length()};
        const auto frame_id = FrameId::from_stdid(frame.identifier().to_stdid());
        const auto axis_id = frame_id.axis_id;
        const auto command = frame_id.command;
        // const auto payload_bytes = std::span(frame.payload().u8x8());
        
        // const auto payload_bytes = frame.payload_bytes_fixed<8>();
        const auto payload_bytes = std::span(frame.payload().u8x8());
        switch(command.kind()){
            case Command::Undefined:{
                //nothing
            }break;
            case Command::Heartbeat:{
                const auto either_msg = resp_msgs::HeartbeatV513::try_from_bytes(payload_bytes);
                return accept_either_msg(axis_id, either_msg);
                // return accept_either_msg(axis_id, de_msg(payload_bytes));
            }break;
            case Command::Estop:{
                //req only
            }break;
            case Command::GetError:{
                const auto either_msg = resp_msgs::GetError::try_from_bytes(payload_bytes);
                return accept_either_msg(axis_id, either_msg);
            }break;
            case Command::RxSdo:{
                //do not handle currently
            }break;
            case Command::TxSdo:{
                //do not handle currently
            }break;
            case Command::SetAxisNodeId:{
                //req only
            }break;
            case Command::SetAxisState:{
                //req only
            }break;
            case Command::MitControl:{
                //do not handle currently
            }break;
            case Command::GetEncoderEstimates:{
                const auto either_msg = resp_msgs::GetEncoderEstimates::try_from_bytes(payload_bytes);
                return accept_either_msg(axis_id, either_msg);
            }break;
            case Command::GetEncoderCount:{
                const auto either_msg = resp_msgs::GetEncoderCount::try_from_bytes(payload_bytes);
                return accept_either_msg(axis_id, either_msg);
            }break;
            case Command::SetControllerMode:{
                //req only
            }break;
            case Command::SetInputPosition:{
                //req only
            }break;
            case Command::SetInputVelocity:{
                //req only
            }break;
            case Command::SetInputTorque:{
                //req only
            }break;
            case Command::SetLimits:{
                //req only
            }break;
            case Command::StartAnticogging:{
                //req only
            }break;
            case Command::SetTrajVelLimit:{
                //req only
            }break;
            case Command::SetTrajAccelLimits:{
                //req only
            }break;
            case Command::SetTrajInertia:{
                //req only
            }break;
            case Command::GetIq:{
                const auto either_msg = resp_msgs::GetIq::try_from_bytes(payload_bytes);
                return accept_either_msg(axis_id, either_msg);
            }break;
            case Command::Reboot:{
                //req only
            }break;
            case Command::GetBusVoltageCurrent:{
                const auto either_msg = resp_msgs::GetBusVoltageCurrent::try_from_bytes(payload_bytes);
                return accept_either_msg(axis_id, either_msg);
            }break;
            case Command::ClearErrors:{
                //req only
            }break;
            case Command::SetMoveIncremental:{
                //req only
            }break;
            case Command::SetPosGain:{
                //req only
            }break;
            case Command::SetVelGain:{
                //req only
            }break;
            case Command::GetTorques:{
                const auto either_msg = resp_msgs::GetTorques::try_from_bytes(payload_bytes);
                return accept_either_msg(axis_id, either_msg);
            }break;
            case Command::GetPowers:{
                const auto either_msg = resp_msgs::GetPowers::try_from_bytes(payload_bytes);
                return accept_either_msg(axis_id, either_msg);
            }break;
            case Command::DisableCan:{
                //req only
            }break;
            case Command::SaveConfig:{
                //req only
            }break;
        }
    };

    [[maybe_unused]] auto write_can_frame = [](const hal::BxCanFrame & frame, const Milliseconds delay_ms = 0ms){
        can.try_write(frame).examine();
        if(delay_ms != 0ms) clock::delay(delay_ms);
    };

    FrameFactory left_factory{AxisId::from_bits(0x01)};
    FrameFactory right_factory{AxisId::from_bits(0x02)};

    clock::delay(100ms);




    #if 1
    [[maybe_unused]] auto & timer = hal::timer2;
    //配置定时器
    timer.init({
        .remap = hal::TIM2_REMAP_A0_A1_A2_A3,
        .count_freq = hal::NearestFreq(500), 
        .count_mode = hal::TimerCountMode::Up
    })  .unwrap()
        .dont_alter_to_pins()
    ;



    //设置定时器事件回调
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            const auto now_secs = clock::seconds();

            {
                const auto num_rx_frames = can.available();
                for(size_t i = 0; i < num_rx_frames; ++i){
                    parse_can_frame(can.read());
                    // (void)(can.read());
                }
            }

            const auto left_torque_ff = iq16(math::sin(now_secs)) / 10;
            const auto right_torque_ff = iq16(math::cos(now_secs)) / 10;

            {
                const auto frame = left_factory.set_input_torque({
                    .torque_ff = float(left_torque_ff)
                });
                write_can_frame(frame);
            }



            {
                const auto frame = right_factory.set_input_torque({
                    .torque_ff = float(right_torque_ff)
                });
                write_can_frame(frame);
            }

            break;
        }
        default: break;
        }
    });

    //使能更新事件的中断
    timer.register_nvic<hal::TimerIT::Update>({0, 0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);

    //启动定时器
    timer.start();

    auto setup_motors = [&](const FrameFactory & factory){
        {
            const auto frame = factory.clear_errors();
            write_can_frame(frame, 1ms);
        }

        {
            const auto frame = factory.set_axis_state({
                .axis_state = AxisState::ClosedLoopControl
            });
            write_can_frame(frame, 1ms);
        }

        {
            const auto frame = factory.set_controller_mode({
                .loop_mode = LoopMode::CurrentLoop,
                .input_mode = InputMode::CurrentRamp,
            });
            write_can_frame(frame, 1ms);
        }
    };

    setup_motors(left_factory);
    setup_motors(right_factory);
    #endif
    while(true){
        

        // static constexpr auto frame = serialize_msg_to_can_frame(
        //     AxisId::from_bits(0), 
        //     req_msgs::SetInputPosition{
        //         .input_position = 2.2,
        //         .vel_ff = 0,
        //         .torque_ff = 0,
        //     }
        // );


        // while(can.available()){
        //     parse_can_frame(can.read());
        //     // (void)(can.read());
        // }

        DEBUG_PRINTLN_IDLE(encoder_feedbacks[0], encoder_feedbacks[1]);
        // DEBUG_PRINTLN(frac(now_secs), torque_ff, iq16(math::sin(now_secs)));

        // const auto frame = hal::BxCanFrame::from_parts(
        //     hal::CanStdId::from_u11(0x111),
        //     hal::BxCanPayload::from_list({0x01, 0x02, 0x03, 0x04})
        // );


        // DEBUG_PRINTLN_IDLE(
        //     can.available(), 
        //     hal::PA<12>().read().to_bool(),
        //     hal::PA<11>().read().to_bool(),
        //     can.last_error(),
        //     clock::millis().count(),
        //     can.get_tx_errcnt(),
        //     can.get_rx_errcnt(),
        //     // frame
        // );

        // clock::delay(10ms);
    }
}

