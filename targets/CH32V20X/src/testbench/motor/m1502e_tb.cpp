#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/math/realmath.hpp"
#include "core/async/timer.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/can/hw_singleton.hpp"
#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "robots/vendor/bmkj/m1502e_highlayer.hpp"
#include "robots/vendor/bmkj/m1502e_ser.hpp"
#include "dsp/controller/adrc/linear/ltd2o.hpp"
#include "dsp/controller/adrc/nonlinear/nltd2o.hpp"

using namespace ymd;

using namespace ymd::robots::bmkj;
using namespace ymd::robots::bmkj::m1502e;
using namespace ymd::robots::bmkj::m1502e::primitive;
using namespace ymd::sync;

using namespace ymd::dsp;


using Nltd2o = dsp::adrc::NonlinearTrackingDifferentiator<iq16, 2>;



struct [[nodiscard]] MotorLeso{
public:

    struct [[nodiscard]] Coeffs{
        uq8 b0;
        uq24 dt;
        uq20 g1;
        uq20 g2;
    };

    struct [[nodiscard]] Config{
        uint32_t fs;
        uq16 fc;
        uq8 b0;

        constexpr Coeffs to_coeffs() const{
            auto & self = *this;
            return Coeffs{
                .b0 = self.b0,
                .dt = 1_uq24 / self.fs,
                .g1 = 2 * self.fc,
                .g2 = self.fc * self.fc
            };
        }
    };


    struct [[nodiscard]] State{
        iq16 x1;
        iq16 x2;
    };

    constexpr explicit MotorLeso(const Coeffs & coeffs):
        coeffs_(coeffs){;}

    constexpr State forward(const State & state, const iq16 y, const iq16 u) const {
        return State{
            state.x1 + (state.x2 + coeffs_.b0 * u + coeffs_.g1 * (y - state.x1)) * coeffs_.dt,
            state.x2 + coeffs_.g2 * (y - state.x1) * coeffs_.dt
        };
    }

private:
    using Self = MotorLeso;
    Coeffs coeffs_;
};



struct [[nodiscard]] MotorControlLawModule{
    using Self = MotorControlLawModule;
    static constexpr uq16 MAX_TORQUE = 2_uq16;
    struct Inputs{
        sync::Probe<uq16> kp;
        sync::Probe<uq16> kd;
        sync::Probe<iq16> x1_ref;
        sync::Probe<iq16> x2_ref;
        sync::Probe<iq16> x1_now;
        sync::Probe<iq16> x2_now;
        sync::Probe<iq16> disturbance;
    };

    struct Outputs{
        sync::Cell<iq16> torque_ref;
    };

    Inputs inputs;
    Outputs outputs = {};


    void iterate(){
        const auto x1_contribute = extended_mul(
            inputs.kp.get(), (inputs.x1_ref.get() - inputs.x1_now.get()));
        const auto x2_contribute = extended_mul(
            inputs.kd.get(), (inputs.x2_ref.get() - inputs.x2_now.get()));
        const auto torque_ref = fixed_downcast<16>(x1_contribute + x2_contribute);
        outputs.torque_ref.set(CLAMP(torque_ref, -iq16(MAX_TORQUE), MAX_TORQUE));
        // outputs.torque_ref.set(fixed_downcast<16>(x1_contribute));

        // const auto x1_contribute = (inputs.x1_ref.get() - inputs.x1_now.get());
        // const auto x2_contribute = (inputs.x2_ref.get() - inputs.x2_now.get());
        // outputs.torque_ref.set(CLAMP2(torque_ref, MAX_TORQUE));
        // outputs.torque_ref.set((x1_contribute));
    }
};

struct [[nodiscard]] MotorStateObserveModule{
    using Self = MotorStateObserveModule;
    struct Inputs{
        sync::Probe<iq16> torque_ref;
    };

    struct Outputs{
        sync::Cell<iq16> x1;
        sync::Cell<iq16> x2;
        sync::Cell<iq16> x3;
        sync::Cell<Angular<uq16>> rotor_lap_angle;
        sync::Cell<iq16> disturbance;
        sync::Cell<iq16> current_amps;
    };

    Inputs inputs;
    Outputs outputs = {};

    struct [[nodiscard]] Feedback{
        Angular<uq32> lap_angle;
        iq16 speed_rps;
        iq16 current_amps;
    };

    void iterate(const Feedback & feedback){
        const auto next_rotor_lap_angle = feedback.lap_angle.cast_inner<uq16>();
        const auto rotor_lap_angle = outputs.rotor_lap_angle.get().cast_inner<iq16>();

        const auto diff_angle = (next_rotor_lap_angle.cast_inner<iq16>() 
            - rotor_lap_angle).signed_normalized();

        outputs.rotor_lap_angle.set(next_rotor_lap_angle);
        outputs.x1.set(outputs.x1.get() + diff_angle.to_turns());
        outputs.x2.set(feedback.speed_rps);
        outputs.current_amps.set(feedback.current_amps);
        // feedback_state_ = feedback_differ_.update(feedback_state_, rotor_multi_angle_.to_turns());
        
        // const auto sensored_elec_angle = ((next_rotor_lap_angle * POLE_PAIRS) + ANGLE_BASE).normalized(); 

    }
};

struct CtrlSystemOutput{
    Probe<iq16> left_torque;
    Probe<iq16> right_torque;
};

static constexpr size_t CTRL_FREQ = 500;

void m1502e_main(){
    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(DEBUG_UART_BAUD),
        .rx_strategy = CommStrategy::Dma,
        .tx_strategy = CommStrategy::Dma
    });

    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);

    auto led = hal::PC<14>();
    led.outpp(HIGH);

    auto & timer = hal::timer2;
    auto & can = hal::can1;


    can.init({
        .remap = hal::CAN1_REMAP_PB9_PB8,//tx:b9 rx:b8
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanBaudrate(hal::CanBaudrate::_500K)
    });

    can.filters<0>().apply(
        hal::CanFilterConfig::accept_all()
    );

    Cell<iq16> left_torque;
    Cell<iq16> right_torque;

    CtrlSystemOutput ctl_sys_output{
        .left_torque = left_torque.probe(),
        .right_torque = right_torque.probe()
    };

    MotorStateObserveModule left_motor_state_observe{
        .inputs = {
            .torque_ref = left_torque.probe()
        }
    };

    MotorStateObserveModule right_motor_state_observe{
        .inputs = {
            .torque_ref = right_torque.probe()
        }
    };

    std::array<Option<LoopMode>, 2> loop_modes{None, None};

    static constexpr auto frame_factory = m1502e::FrameFactory{};

    auto write_can_frame = [](const hal::BxCanFrame & frame){
        can.try_write(frame).examine();
    };

    auto motor_pusblish_guard_isr = [&]{
        auto check_mode_is_currentloop = [&](const Option<LoopMode> & mode){
            return mode.is_some() and mode.unwrap() == LoopMode::CloseloopCurrent;
        };

        auto guard_closeloop_mode = [&]{
            //监视电机的闭环模式必须为电流闭环模式，否则进行纠正
            const auto en1 = check_mode_is_currentloop(loop_modes[0]);
            const auto en2 = check_mode_is_currentloop(loop_modes[1]);
            if(not(en1 || en2)) return;
            const auto frame = frame_factory.set_loop_mode(req_msgs::SetLoopMode::from_all(LoopMode::CloseloopCurrent));
            write_can_frame(frame);
        };

        guard_closeloop_mode();

    };


    auto motor_pusblish_torque_isr = [&]{
        const auto left_current_amps = ctl_sys_output.left_torque.get();
        const auto right_current_amps = - ctl_sys_output.right_torque.get();

        const auto frame = frame_factory.set_low_quad_motor_setpoint({
            .setpoints = {
                CurrentCode::from_amps(left_current_amps).unwrap(), 
                CurrentCode::from_amps(right_current_amps).unwrap(), 
                0, //未用到三号电机
                0  //未用到四号电机
            }
        });

        write_can_frame(frame);
    };

    auto body_ctl_iterate_isr = [&]{ 

        // track_ref_ = command_shaper_.update(track_ref_, x1_input.get(), x2_input.get());
        // x1_ref.set(fixed_downcast<16>(track_ref_.x1));
        // x2_ref.set(track_ref_.x2);
        // motor_control_law_module.iterate();
    };

    auto handle_motor_feedback_msg = [&](const resp_msgs::StateFeedback & msg){
        [[maybe_unused]] const auto motor_id = msg.motor_id;
        switch(motor_id.nth().count()){
            case 1:
                left_motor_state_observe.iterate({
                    .lap_angle = msg.lap_position.to_angle(),
                    .speed_rps = msg.speed.to_rps(),
                    .current_amps = msg.current.to_amps()
                });
                loop_modes[0] = Some(msg.loop_mode);
                break;
            case 2:
                right_motor_state_observe.iterate({
                    .lap_angle = msg.lap_position.to_angle(),
                    .speed_rps = msg.speed.to_rps(),
                    .current_amps = msg.current.to_amps()
                });
                loop_modes[1] = Some(msg.loop_mode);
                break;
            default:
                PANIC{"unknown motor id"};
        }
    };

    auto handle_received_frame = [&](const hal::BxCanFrame & frame) -> void {
        
        //  请求的CANID       响应的CANID
        //  0x32             0x96   +   MOTOR_ID           设置低四个电机的参数
        //  0x33             0x96   +   MOTOR_ID           设置高四个电机的参数
        //  0x105            0x200  +   MOTOR_ID           设置环路模式
        //  0x106            0x264  +   MOTOR_ID           设置反馈策略
        //  0x107            0x96   +   MOTOR_ID           查询三项内容
        //  0x108            0x96   +   MOTOR_ID           设置MOTOR_ID
        //  0x109            0x390  +   MOTOR_ID           设置CAN终端电阻
        //  0x10A            0x2C8  +   MOTOR_ID           设置通信超时读写操作
        //  0x10A            0x32C  +   MOTOR_ID           查询固件版本
        
        if(frame.is_extended()) {
            PANIC{"received extended frame"};
        }
        if(frame.is_remote()) {
            PANIC{"received remote frame"};
        }

        const auto id_u11 = static_cast<uint16_t>(frame.id_u32());

        switch(id_u11){
        case (0x96 + 1) ... (0x96 + 8):
            //普通反馈帧
            {
                const auto either_resp_msg = resp_msgs::StateFeedback::try_from_can_frame(frame);
                if(either_resp_msg.is_ok()){
                    const auto resp = either_resp_msg.unwrap();
                    handle_motor_feedback_msg(resp);
                };
            }
            break;
        case (0x200 + 1) ... (0x200 + 8):
            //设置环路模式
            {
            }
            break;
        case (0x264 + 1) ... (0x264 + 8):
            //设置反馈策略
            {
            }
            break;
        case (0x390 + 1) ... (0x390 + 8):
            //设置CAN终端电阻
            {
            }
            break;
        case (0x2c8 + 1) ... (0x2c8 + 8):
            //设置通信超时读写操作
            {
            }
            break;
        case (0x32c + 1) ... (0x32c + 8):
            // 查询固件版本
            {
                const auto either_resp_msg = resp_msgs::QueryFirmwareVersion::try_from_can_frame(frame);
                if(either_resp_msg.is_ok()){
                    const auto resp = either_resp_msg.unwrap();
                    DEBUG_PRINTLN(
                        resp
                    );
                };
            }
            break;
        }
        return;
    };

    timer.init({
        .remap = hal::TimerRemap::_0,
        .count_freq = hal::NearestFreq(CTRL_FREQ), 
        .count_mode = hal::TimerCountMode::Up
    }).unwrap().dont_alter_to_pins();


    timer.register_nvic<hal::TimerIT::Update>({0, 0}, EN);

    timer.set_event_callback([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            motor_pusblish_guard_isr();
            motor_pusblish_torque_isr();
            body_ctl_iterate_isr();
            break;
        }
        default: break;
        }
    });

    #if 0
    // 已经设置完电机ID,不需要再用这段代码
    // write_can_frame(req_msgs::SetMotorId{MotorId::from_bits(0x02)}.to_can_frame());
    #endif 


    clock::delay(10ms);
    // write_can_frame(req_msgs::SetLoopMode::from_all(LoopMode::CloseloopCurrent).to_can_frame());
    write_can_frame(frame_factory.set_loop_mode(req_msgs::SetLoopMode::from_all(LoopMode::CloseloopCurrent)));
    clock::delay(10ms);
    write_can_frame(frame_factory.set_loop_mode(req_msgs::SetLoopMode::from_all(LoopMode::CloseloopCurrent)));

    // write_can_frame(req_msgs::SetFeedbackStrategy::from_all(
    //     FeedbackStrategy::from_duration(1ms).unwrap()).to_can_frame());
    // clock::delay(10ms);

    timer.enable_interrupt<hal::TimerIT::Update>(EN);

    while(true){
        const auto now_secs = clock::seconds();

        // kp.set(20);
        // kd.set(4.7_uq16);
        [[maybe_unused]] const auto omega = 2.2_iq16;
        [[maybe_unused]] const auto amplitude = 0.8_iq16;

        [[maybe_unused]] const auto [s,c] = math::sincos(omega * now_secs);
        // x1_input.set(amplitude * frac(omega * now_secs));
        // x2_input.set(amplitude * omega);
        left_torque.set(amplitude * iq16(s));
        right_torque.set(amplitude * iq16(c));
        // left_torque.set(0.2_iq16);
        // right_torque.set(0.2_iq16);

        if(can.available()){
            const auto frame = can.read();
            handle_received_frame(frame);
        }
        static auto report_timer = async::RepeatTimer::from_duration(2ms);
        
        report_timer.invoke_if([&]{

            const auto & left_states = left_motor_state_observe.outputs;
            const auto & right_states = right_motor_state_observe.outputs;


            DEBUG_PRINTLN_IDLE(
                // cnt_.load()
                left_states.x1,
                left_states.x2,
                left_states.rotor_lap_angle.get().to_turns(),
                
                right_states.x1,
                right_states.x2,
                right_states.rotor_lap_angle.get().to_turns()
                // x1_ref,
                // x2_ref,
                // motor_control_law_module.outputs.torque_ref.get()
                // outputs.current_amps,
                // motor_state_observe_module.inputs.torque_ref.get()
                
            );
        });
    }
}