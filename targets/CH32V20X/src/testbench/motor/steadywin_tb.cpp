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
#include "middlewares/reactive/cellprobe.hpp"
#include "robots/kinematics/differential_drive/differential_drive_kinematics.hpp"

#include "middlewares/repl/repl.hpp"
#include "middlewares/repl/repl_server.hpp"
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

static constexpr uint8_t LEFT_MOTOR_AXIS_ID = 0x02;
static constexpr uint8_t RIGHT_MOTOR_AXIS_ID = 0x01;

static constexpr auto TRACK_WIDTH = 0.67_iq16;
static constexpr auto WHEEL_RADIUS = 0.14_iq16;


//纵向驱动加速度约束（米/秒^2)
static constexpr iq16 BODY_LINEAR_X3_DRIVE_LIMIT = 0.35_iq16;

//纵向刹车加速度约束（米/秒^2)
static constexpr iq16 BODY_LINEAR_X3_BRAKE_LIMIT = 1.05_iq16;

constexpr auto BODY_KINEMATICS = ymd::robots::kinematics::DifferentialDriveKinematics::Config{
    .wheel_radius = WHEEL_RADIUS,
    .track_width = TRACK_WIDTH
}.into();



//对步进进行限幅 绝对值增长时使用arise_max_step 绝对值减少时使用decay_max_step
template<typename T>
static constexpr T step_to_ad(const T now, const T target, const T arise_max_step, const T decay_max_step){
    T diff = target - now;
    T abs_diff = diff >= 0 ? diff : -diff;

    // now 和 diff 同号(或 now=0) 时绝对值增长，异号时绝对值减少
    T max_step = (now * diff >= 0) ? arise_max_step : decay_max_step;

    if (abs_diff > max_step) {
        return now + (diff >= 0 ? max_step : -max_step);
    }
    return target;
}

//绝对值增长，目标在限制范围内
static_assert(std::abs(step_to_ad(0.0, 1.0, 2.0, 0.3) - 1.0) < 1e-6);
static_assert(std::abs(step_to_ad(0.0, -1.0, 2.0, 0.3) - -1.0) < 1e-6);

//绝对值增长，需要限制步长（正向）
static_assert(std::abs(step_to_ad(0.0, 10.0, 2.0, 0.3) - 2.0) < 1e-6);

//绝对值增长，需要限制步长（负向）
static_assert(std::abs(step_to_ad(0.0, -10.0, 2.0, 0.3) - -2.0) < 1e-6);

//绝对值减少（正向减少）
static_assert(std::abs(step_to_ad(10.0, 5.0, 2.0, 3.0) - 7.0) < 1e-6);

//绝对值减少，需要限制步长（正向）
static_assert(std::abs(step_to_ad(10.0, 0.0, 2.0, 3.0) - 7.0) < 1e-6);

//绝对值减少（负向减少）
static_assert(std::abs(step_to_ad(-10.0, -5.0, 2.0, 3.0) - -7.0) < 1e-6);

//绝对值减少，需要限制步长（负向）
static_assert(std::abs(step_to_ad(-10.0, 0.0, 2.0, 3.0) - -7.0) < 1e-6);

//已在目标值
static_assert(std::abs(step_to_ad(5.0, 5.0, 2.0, 0.3) - 5.0) < 1e-6);

//跨越零点
static_assert(std::abs(step_to_ad(-2.0, 2.0, 2.0, 0.3) - -1.7) < 1e-6);

//横向加速度约束（弧度/秒^2)
static constexpr auto BODY_ANGULAR_X3_DRIVE_LIMIT = ymd::Angular<iq16>::from_radians(3.61_iq16);
static constexpr auto BODY_ANGULAR_X3_BRAKE_LIMIT = ymd::Angular<iq16>::from_radians(4.81_iq16);

//横向速度约束（弧度/秒)
static constexpr auto BODY_ANGULAR_X2_LIMIT = ymd::Angular<iq16>::from_radians(1.61_iq16);

//#region 电机控制器参数
//控制频率
static constexpr size_t MOTOR_CTRL_FREQ = 1000;
static constexpr iq16 MOTOR_CTRL_DELTA_TIME = 1_iq16 / MOTOR_CTRL_FREQ;


void steadywin_main(){
    auto & DBG_UART = hal::usart2;
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
        .tx_strategy = CommStrategy::Blocking 
    });

    auto & COMM_UART = hal::uart4;
    COMM_UART.init({
        .remap = hal::UART4_REMAP_PB0_PB1,
        .baudrate = hal::NearestFreq(921600),
        .tx_strategy = CommStrategy::Blocking 
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.build_config()
        .set_eps(4)
        .set_splitter(",")
        .no_brackets(EN)
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();

    auto & can = hal::can1;
    //初始化CAN外设
    can.init({
        //映射到TX:PA12 RX:PA11
        .remap = hal::CAN1_REMAP_PB9_PB8,
        .wiring_mode = hal::CanWiringMode::Normal,
        //波特率为1M
        .bit_timming = hal::CanNominalBitTimmingCoeffs{
            .prescale = 9,
            .swj = hal::CanTq::from_num(2),
            .bs1 = hal::CanTq::from_num(12),
            .bs2 = hal::CanTq::from_num(3)
        },  
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
        if(frame.is_extended()) PANIC{"ext can frame received!!"};
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
        .count_freq = hal::NearestFreq(1000), 
        .count_mode = hal::TimerCountMode::Up
    })  .unwrap()
        .dont_alter_to_pins()
    ;


    sync::AtomicCell<Angular<iq16>> ramped_body_angular_x2_;
    sync::AtomicCell<iq16> ramped_body_linear_x2_;

    sync::AtomicCell<Angular<iq16>> constrained_body_angular_x2_;
    sync::AtomicCell<iq16> constrained_body_linear_x2_;

    auto on_mc_callback = [&]{
        ramped_body_linear_x2_.set(step_to_ad(
            ramped_body_linear_x2_.get(), 
            constrained_body_linear_x2_.get(), 
            iq16(BODY_LINEAR_X3_DRIVE_LIMIT * MOTOR_CTRL_DELTA_TIME),
            iq16(BODY_LINEAR_X3_BRAKE_LIMIT * MOTOR_CTRL_DELTA_TIME)
        ));

        ramped_body_angular_x2_.set(make_angular_from_turns(step_to_ad(
            ramped_body_angular_x2_.get().to_turns(),
            constrained_body_angular_x2_.get().to_turns(), 
            (BODY_ANGULAR_X3_DRIVE_LIMIT * MOTOR_CTRL_DELTA_TIME).to_turns(),
            (BODY_ANGULAR_X3_BRAKE_LIMIT * MOTOR_CTRL_DELTA_TIME).to_turns()
        )));

        const auto [desired_left_motor_x2, desired_right_motor_x2] = BODY_KINEMATICS.inverse(
            ramped_body_linear_x2_.get(),
            ramped_body_angular_x2_.get()
        );



        static constexpr float F32_ZERO = 0;
        static constexpr size_t DEDUCTION_RATIO = 8;
        {
            const auto frame = left_factory.set_input_velocity({
                .vel_ff = static_cast<float>(-desired_left_motor_x2.to_turns() * DEDUCTION_RATIO),
                // .vel_ff = static_cast<float>(0),
                .torque_ff = F32_ZERO
            });
            write_can_frame(frame);
        }



        {
            const auto frame = right_factory.set_input_velocity({
                .vel_ff = static_cast<float>(desired_right_motor_x2.to_turns() * DEDUCTION_RATIO),
                .torque_ff = F32_ZERO
            });
            write_can_frame(frame);
        }


        while(true){
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
    };

    //设置定时器事件回调
    timer.set_event_callback([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            on_mc_callback();
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
                .loop_mode = LoopMode::VelocityLoop,
                .input_mode = InputMode::PassThrough,
            });
            write_can_frame(frame, 1ms);
        }
    };

    setup_motors(left_factory);
    setup_motors(right_factory);

    auto led_pin = hal::PC<13>();
    led_pin.outpp();




    [[maybe_unused]] auto poll_repl_activity = [&]{
        [[maybe_unused]] static repl::ReplServer repl_server{&COMM_UART, &COMM_UART};
        repl_server.enable_echo(DISEN);

        [[maybe_unused]] static const auto list = script::make_list(
            "list",
            script::make_function("rst", [](){sys::reset();}),
            script::make_function("ctl", [&](
                const iq16 lx2, 
                const iq16 lx3,
                const iq16 ax2, 
                const iq16 ax3
            ){
                // DEBUG_PRINTLN(lx2);
                constrained_body_linear_x2_.set(CLAMP2(lx2, 2.0_iq16));
                constrained_body_angular_x2_.set(Angular<iq16>::from_radians(ax2));
            })

            // script::make_function("w", [&](
            //     const StringView str
            // ){
            //     DEBUG_PRINTLN(strconv::FstrDump::parse(str));
            // })

        );

        repl_server.invoke(list);
    };
    
    while(true){
        led_pin.write(BoolLevel::from((static_cast<uint32_t>(clock::millis().count()) % 200 -100) > 0));
        //modify to your own controller
        poll_repl_activity();
        // DEBUG_PRINTLN_IDLE(encoder_feedbacks_[0], encoder_feedbacks_[1]);

    }
}

