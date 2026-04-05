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

#include "robots/vendor/steadywin/can_simple/steadywin_can_simple_msgs.hpp"
#include "robots/vendor/steadywin/can_simple/steadywin_can_simple_factory.hpp"

#include "drivers/common_io/Key/Key.hpp"

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
        return os 
            << os.field("multilap_angle")(self.multilap_angle.to_turns()) << os.splitter()
            << os.field("lap_angle")(self.lap_angle.to_turns());
    }
};

static constexpr uint8_t LEFT_MOTOR_AXIS_ID = 0x01;
static constexpr uint8_t RIGHT_MOTOR_AXIS_ID = 0x02;

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
        .bit_timming = hal::CanNominalBitTimming(hal::CanBaudrate::_1M), 
    });
    
    //配置can过滤器为接收标准数据帧（滤除拓展和远程帧）
    can.configure_filter(
        0_nth, 
        hal::CanFifoIndex::_0,
        hal::CanFilterConfig::from_pairs(
            hal::CAN_FILTER_PAIR_STD_DATA_FRAME_ONLY,
            hal::CanStdIdMaskPair::reject_all()
        )
    ).unwrap();

    std::array<EncoderFeedback, 2> encoder_feedbacks_ = {Zero, Zero};

    auto axis_id_to_idx = [](const AxisId axis_id) -> size_t {
        switch(axis_id.to_bits()){
            case LEFT_MOTOR_AXIS_ID: return 0;
            case RIGHT_MOTOR_AXIS_ID: return 1;
        }
        PANIC{};
    };
    auto store_encoder_feedback = [&](const size_t idx, const resp_msgs::GetEncoderCount & msg){
        encoder_feedbacks_.at(idx) = EncoderFeedback{
            .multilap_angle = msg.multilap_angle,
            .lap_angle = msg.lap_angle
        };
    };

    [[maybe_unused]] auto handle_can_frame = [&](const hal::ClassicCanFrame & frame){
        if(frame.is_extended()) PANIC{"std can frame received!!"};
        if(frame.length() != 8) PANIC{"can frame length is not 8", frame.length()};
        const auto frame_id = FrameId::from_stdid(frame.identifier().to_stdid());

        const auto axis_id = frame_id.axis_id;
        const auto command = frame_id.command;

        const auto u8x8 = std::span(frame.payload().u8x8);
        switch(command.kind()){
            case Command::Nop:{
                //nothing
            }break;
            case Command::Heartbeat:{
                //not used
            }break;
            case Command::Estop:{
                //req only
            }break;
            case Command::GetError:{
                //do not handle currently
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
                const auto either_msg = resp_msgs::GetEncoderEstimates::try_from_bytes(u8x8);
                if(not either_msg.is_ok()) return;
                const resp_msgs::GetEncoderEstimates msg = either_msg.unwrap();
                (void)(msg.position);
                (void)(msg.velocity);
                // DEBUG_PRINTLN(msg.position, msg.velocity);
            }break;
            case Command::GetEncoderCount:{
                const auto either_msg = resp_msgs::GetEncoderCount::try_from_bytes(u8x8);
                if(not either_msg.is_ok()) return;
                const resp_msgs::GetEncoderCount msg = either_msg.unwrap();
                (void)(msg.lap_angle);
                (void)(msg.multilap_angle);
                // DEBUG_PRINTLN(msg.lap_angle.to_turns(), msg.multilap_angle.to_turns());
                store_encoder_feedback(axis_id_to_idx(axis_id), msg);
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
                //not used
            }break;
            case Command::Reboot:{
                //req only
            }break;
            case Command::GetBusVoltageCurrent:{
                //not used
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
                //not used
            }break;
            case Command::GetPowers:{
                //not used
            }break;
            case Command::DisableCan:{
                //req only
            }break;
            case Command::SaveConfig:{
                //req only
            }break;
        }
    };

    [[maybe_unused]] auto write_can_frame = [](const hal::ClassicCanFrame & frame, const Milliseconds delay_ms = 0ms){
        can.try_write(frame).examine();
        if(delay_ms != 0ms) clock::delay(delay_ms);
    };

    static constexpr FrameFactory left_factory{AxisId::from_bits(LEFT_MOTOR_AXIS_ID)};
    static constexpr FrameFactory right_factory{AxisId::from_bits(RIGHT_MOTOR_AXIS_ID)};

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


    iq16 left_torque_ff_ = 0;
    iq16 right_torque_ff_ = 0;


    //设置定时器事件回调
    timer.set_event_callback([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{



            {
                const auto can_frame = ({
                    if(can.available() == 0){
                        //cant read more can frame;
                        break;
                    }

                    const auto may_frame = can.try_read();
                    if(may_frame.is_none()){
                        //cant read more can frame;
                        break;
                    }
                    may_frame.unwrap();
                });
                handle_can_frame(can_frame);
            }


            {
                const auto frame = left_factory.set_input_torque({
                    .torque_ff = static_cast<float>(left_torque_ff_)
                });
                write_can_frame(frame);
            }



            {
                const auto frame = right_factory.set_input_torque({
                    .torque_ff = static_cast<float>(right_torque_ff_)
                });
                write_can_frame(frame);
            }

            break;
        }
        default: break;
        }
    });

    //使能更新事件的中断
    timer.register_nvic<hal::TimerIT::Update>(hal::NvicPriorityCode::highest(), EN);
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


    while(true){
        const auto now_secs = clock::seconds();

        //modify to your own controller

        left_torque_ff_ = iq16(math::sin(now_secs)) / 10;
        right_torque_ff_ = iq16(math::cos(now_secs)) / 10;

        DEBUG_PRINTLN_IDLE(encoder_feedbacks_[0], encoder_feedbacks_[1]);

    }
}

