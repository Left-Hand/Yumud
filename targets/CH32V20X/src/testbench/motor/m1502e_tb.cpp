#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/math/realmath.hpp"
#include "core/sync/timer.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "middlewares/reactive/cellprobe.hpp"
#include "robots/vendor/bmkj/m1502e_msgs.hpp"
#include "dsp/motor_ctrl/tracking_differentiator.hpp"

using namespace ymd;

using namespace ymd::robots::bmkj;
using namespace ymd::robots::bmkj::m1502e;
using namespace ymd::robots::bmkj::m1502e::primitive;
using namespace ymd::reactive;

using namespace ymd::dsp;


template<size_t Q_to, size_t Q_from, typename D>
static constexpr fixed_t<Q_to, int32_t> fixed_downcast(const fixed_t<Q_from, D> val){
    return fixed_t<Q_to, int32_t>::from_bits(static_cast<int32_t>(val.to_bits() >> (Q_from - Q_to)));
}
// static_assert(std::is_same_v<int64_t, long long int>);


struct motor_recipes{
    struct Timeout{};
    struct ShutDown{};
    struct SendFrame{const hal::CanClassicFrame frame;};
    struct PropagationException{};
};

struct [[nodiscard]] MotorHealthGuard{
    using CanFrame = m1502e::CanFrame;
    using Exception = m1502e::Exception;

    struct [[nodiscard]]Recipe:public Sumtype<
        motor_recipes::Timeout,
        motor_recipes::ShutDown,
        motor_recipes::SendFrame,
        motor_recipes::PropagationException
    >{

    };


    struct [[nodiscard]]Switches{
        struct{
            uint16_t shutdown_on_fault:1;
        } fault_protect;
    };


    struct [[nodiscard]]Config{
        Switches switches;
        m1502e::LoopMode loop_mode;
        uint8_t report_duration_ms;
        uint16_t timeout_ms;
    };

    explicit constexpr MotorHealthGuard(const Config & cfg): 
        cfg_(cfg){;}


    struct FeedBack{
        m1502e::ExceptionCode exception_code;
        m1502e::LoopMode loop_mode;
    };

    constexpr Option<Recipe> poll(const Milliseconds now) {
        //使用卫语句优先处理更严重的问题

        if(health_.may_exception.is_none() == false){
            //电机发生了异常
            const auto exception = health_.may_exception.unwrap();
            return handle_exception(exception);
        }

        {//心跳检查
            if(health_.may_last_heartbeat_tick.is_none()){
                //没有心跳过 以第一次侦测开始计时
                health_.may_last_heartbeat_tick = Some(now);
            }else{
                const auto last_heartbeat_tick = health_.may_last_heartbeat_tick.unwrap();
            }
        }
    }
private:
    Config cfg_;

    struct [[nodiscard]] MotorHealth{
        using Self = MotorHealth;
        Option<m1502e::Exception> may_exception;
        Option<m1502e::LoopMode> loop_mode;
        Option<Milliseconds> may_last_heartbeat_tick;

        static constexpr Self from_default(){
            return Self{
                .may_exception = None,
                .loop_mode = None,
                .may_last_heartbeat_tick = None
            };
        }
    };


    MotorHealth health_ = MotorHealth::from_default();

    constexpr Option<Recipe> handle_exception(const Exception exception) {
        [[maybe_unused]] const auto & switches = cfg_.switches;
        switch(exception){
            case Exception::UnderVoltage2:
                // 电压小于17V
                break;
            case Exception::UnderVoltage1:
                // 17V - 22V
                break;
            case Exception::OverVoltage:
                // 36V +
                break;
            case Exception::OverCurrent:
                // 15A +
                break;
            case Exception::OverSpeed:
                //250RPM +
                break;
            case Exception::OverHeat2:
                // 120C +
                break;
            case Exception::OverHeat1:
                // 80C +
                break;
            case Exception::EncoderError:

                break;
            case Exception::EncoderSingalCorrupted:

                break;
            case Exception::CommunicationTimeout:

                break;
            case Exception::Stall:

                break;
        }
        return None;
    }
};


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


struct MotorStateObserveModule{
    using Self = MotorStateObserveModule;
    struct Inputs{
        reactive::Probe<iq16> torque_ref;
    };

    struct Outputs{
        reactive::Cell<iq16> x1;
        reactive::Cell<iq16> x2;
        reactive::Cell<iq16> x3;
        reactive::Cell<Angle<uq16>> rotor_lap_angle;
        reactive::Cell<iq16> disturbance;
        reactive::Cell<iq16> current_amps;
    };

    Inputs inputs;
    Outputs outputs = {};

    struct [[nodiscard]] Feedback{
        Angle<uq32> lap_angle;
        iq16 speed_rps;
        iq16 current_amps;
    };

    void iterate(const Feedback & feedback){
        const auto next_rotor_lap_angle = feedback.lap_angle.cast_inner<uq16>();
        const auto rotor_lap_angle = outputs.rotor_lap_angle.get().cast_inner<iq16>();

        const auto diff_angle = (next_rotor_lap_angle.cast_inner<iq16>() 
            - rotor_lap_angle).normalized();

        outputs.rotor_lap_angle.set(next_rotor_lap_angle);
        outputs.x1.set(outputs.x1.get() + diff_angle.to_turns());
        outputs.x2.set(feedback.speed_rps);
        outputs.current_amps.set(feedback.current_amps);
        // feedback_state_ = feedback_differ_.update(feedback_state_, rotor_multi_angle_.to_turns());
        
        // const auto sensored_elec_angle = ((next_rotor_lap_angle * POLE_PAIRS) + ANGLE_BASE).normalized(); 

    }
};

struct MotorControlLawModule{
    using Self = MotorControlLawModule;
    static constexpr uq16 MAX_TORQUE = 2_uq16;
    struct Inputs{
        reactive::Probe<uq16> kp;
        reactive::Probe<uq16> kd;
        reactive::Probe<iq16> x1_ref;
        reactive::Probe<iq16> x2_ref;
        reactive::Probe<iq16> x1_now;
        reactive::Probe<iq16> x2_now;
        reactive::Probe<iq16> disturbance;
    };

    struct Outputs{
        reactive::Cell<iq16> torque_ref;
    };

    Inputs inputs;
    Outputs outputs = {};


    void iterate(){
        const auto x1_contribute = extended_mul(
            inputs.kp.get(), (inputs.x1_ref.get() - inputs.x1_now.get()));
        const auto x2_contribute = extended_mul(
            inputs.kd.get(), (inputs.x2_ref.get() - inputs.x2_now.get()));
        const auto torque_ref = fixed_downcast<16>(x1_contribute + x2_contribute);
        outputs.torque_ref.set(CLAMP2(torque_ref, MAX_TORQUE));
        // outputs.torque_ref.set(fixed_downcast<16>(x1_contribute));

        // const auto x1_contribute = (inputs.x1_ref.get() - inputs.x1_now.get());
        // const auto x2_contribute = (inputs.x2_ref.get() - inputs.x2_now.get());
        // outputs.torque_ref.set(CLAMP2(torque_ref, MAX_TORQUE));
        // outputs.torque_ref.set((x1_contribute));
    }
};

static constexpr size_t CTRL_FREQ = 500;

void m1502e_main(){
    DEBUGGER_INST.init({
        .baudrate = DEBUG_UART_BAUD,
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
        .remap = 1,//tx:b9 rx:b8
        .mode = hal::CanMode::Normal,
        .timming_coeffs = hal::CanBaudrate(hal::CanBaudrate::_500K).to_coeffs(), 
    });

    can.filters<0>().apply(
        hal::CanFilterConfig::accept_all()
    );

    clock::delay(10ms);
    can.write(req_msg::SetLoopMode::from_all(LoopMode::CloseloopCurrent).to_can_frame()).examine();
    can.write(req_msg::SetFeedbackStrategy::from_all(
        FeedbackStrategy::from_duration(1ms).unwrap()).to_can_frame()).examine();
    clock::delay(10ms);


    MotorStateObserveModule motor_state_observe_module{
        .inputs = {
            .torque_ref = Probe<iq16>::zero()
        }
    };

    Cell<iq16> x1_input;
    Cell<iq16> x2_input;
    Cell<iq16> x1_ref;
    Cell<iq16> x2_ref;
    Cell<uq16> kp;
    Cell<uq16> kd;

    static constexpr auto coeffs = typename NonlinearTrackingDifferentor::Config{
        .fs = CTRL_FREQ,
        // .r = 30.5_q24,
        // .h = 2.5_q24
        // .r = 252.5_iq10,
        // .r = 152.5_iq10,
        .r = 5.5_iq10,
        .h = 0.005_iq10,
        .x2_limit = 3.5_iq16
    }.try_to_coeffs().unwrap();

    SecondOrderState track_ref_;

    static NonlinearTrackingDifferentor command_shaper_{
        coeffs
    };


    MotorControlLawModule motor_control_law_module{
        .inputs = {
            .kp = kp.probe(),
            .kd = kd.probe(),
            .x1_ref = x1_ref.probe(),
            .x2_ref = x2_ref.probe(),
            .x1_now = motor_state_observe_module.outputs.x1.probe(),
            .x2_now = motor_state_observe_module.outputs.x2.probe(),
            .disturbance = motor_state_observe_module.outputs.disturbance.probe(),
        }
    };

    motor_state_observe_module.inputs.torque_ref = 
        motor_control_law_module.outputs.torque_ref.probe();

    auto motor_ctl_callback = [&]{
        if(can.pending() != 0) return;

        track_ref_ = command_shaper_.update(track_ref_, x1_input.get(), x2_input.get());
        x1_ref.set(fixed_downcast<16>(track_ref_.x1));
        x2_ref.set(track_ref_.x2);
        motor_control_law_module.iterate();
        // std::vector<hal::CanClassicFrame> frames;
        const auto motor_torque = 
            motor_control_law_module.outputs.torque_ref.get()
            + 0.2_uq16 * x2_ref.get()
            ;
        const auto frame = req_msg::SetLowQuadMotorSetpoint{
            .setpoints = {
                CurrentCode::from_amps(motor_torque).unwrap(), 
                // CurrentCode::from_amps(torque_ref).unwrap(), 
                // CurrentCode::from_amps(torque_ref).unwrap(), 
                // CurrentCode::from_amps(torque_ref).unwrap(), 
                0,0,0
            }
        }.to_can_frame();

        // for(const auto & frame : frames){
        can.write(frame).examine();
        // }
        // DEBUG_PRINTLN(CurrentCode::from_amps(torque_ref).unwrap().to_amps());
    };

    timer.init({
        .count_freq = hal::NearestFreq(CTRL_FREQ), 
        .count_mode = hal::TimerCountMode::Up
    }, EN);

    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.register_nvic<hal::TimerIT::Update>({0, 0}, EN);

    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            motor_ctl_callback();
            break;
        }
        default: break;
        }
    });

    while(true){
        const auto ctime = clock::time();

        kp.set(20);
        kd.set(4.7_uq16);
        const auto omega = 0.2_iq16;
        const auto amplitude = 16_iq16;

        const auto [s,c] = sincos(omega * ctime);
        // x1_ref.set(amplitude * s);
        // x2_ref.set(amplitude * omega * c);
        x1_input.set(amplitude * frac(omega * ctime));
        x2_input.set(amplitude * omega);



        // current_ref2 = 2.0_iq16 * iq16(sin(ctime));

        if(can.available()){
            const auto frame = can.read();

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
            
            [&] -> void {
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
                        const auto either_resp = resp_msg::StateFeedback::from_can_frame(frame);
                        if(either_resp.is_ok()){
                            const auto resp = either_resp.unwrap();

                            motor_state_observe_module.iterate({
                                .lap_angle = resp.position.to_angle(),
                                .speed_rps = resp.speed.to_rps(),
                                .current_amps = resp.current.to_amps()
                            });
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
                        const auto either_resp = resp_msg::QueryFirmwareVersion::from_can_frame(frame);
                        if(either_resp.is_ok()){
                            const auto resp = either_resp.unwrap();
                            DEBUG_PRINTLN(
                                resp
                                // resp.position.to_angle().cast_inner<uq16>(), 
                                // resp.current.to_amps(),
                                // static_cast<uint8_t>(resp.loop_mode)
                            );
                        };
                    }
                    break;
                }
                return;
            }();
        }
        static auto report_timer = async::RepeatTimer::from_duration(2ms);
        
        report_timer.invoke_if([&]{

            // DEBUG_PRINTLN(
            //     // resp.position.to_angle().cast_inner<uq16>().to_turns(), 
            //     resp.position.to_angle().to_turns(), 
            //     resp.speed.to_rpm(),
            //     // resp.position.bits,
            //     CurrentCode::from_amps(torque_ref.get()).unwrap().to_amps(),
            //     resp.current.to_amps(),
            //     static_cast<uint8_t>(resp.loop_mode),
            //     resp.exception_code.to_bits()
            // );
            auto & outputs = motor_state_observe_module.outputs;


            DEBUG_PRINTLN_IDLE(
                // cnt_.load()
                outputs.x1,
                outputs.x2,
                outputs.rotor_lap_angle.get().to_turns(),
                x1_ref,
                x2_ref,
                motor_control_law_module.outputs.torque_ref.get()
                // outputs.current_amps,
                // motor_state_observe_module.inputs.torque_ref.get()
                
            );
        });
    }
}