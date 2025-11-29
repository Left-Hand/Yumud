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

using namespace ymd;

using namespace ymd::robots::bmkj;
using namespace ymd::robots::bmkj::m1502e;
using namespace ymd::robots::bmkj::m1502e::primitive;
using namespace ymd::reactive;



struct MotorFeedbackState{
    Angle<uq32> lap_angle;
    iq16 current;
};


struct MotorHealth{
    m1502e::ExceptionCode may_exception;
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


struct MotorStateObserverModule{
    using Self = MotorStateObserverModule;
    struct Inputs{
        reactive::Probe<iq16> torque_cmd;
    };

    struct Outputs{
        reactive::Cell<iq16> x1;
        reactive::Cell<iq16> x2;
        reactive::Cell<iq16> x3;
        reactive::Cell<iq16> disturbance;
    };

    void iterate(const MotorFeedbackState & feedback){
        // const auto openloop_manchine_angle = Angle<iq16>::from_turns(0 * ctime);
        // // const auto openloop_manchine_angle = Angle<iq16>::from_turns(1.2_r * ctime);
        // // const auto openloop_manchine_angle = Angle<iq16>::from_turns(sinpu(0.2_r * ctime));
        // const auto openloop_elec_angle = openloop_manchine_angle * POLE_PAIRS;

        // static constexpr auto ANGLE_BASE = Angle<uq16>::from_turns(0.78_uq16);
        // const auto next_encoder_lap_angle = mt6825_.get_lap_angle().examine().cast_inner<uq16>();

        // // const auto diff_angle = (next_encoder_lap_angle.cast_inner<iq16>() 
        // diff_angle = (next_encoder_lap_angle.cast_inner<iq16>() 
        //     - encoder_lap_angle_.cast_inner<iq16>()).normalized();

        // encoder_lap_angle_ = next_encoder_lap_angle;
        // encoder_multi_angle_ = encoder_multi_angle_ + diff_angle;
        // feedback_state_ = feedback_differ_.update(feedback_state_, encoder_multi_angle_.to_turns());
        
        // const auto sensored_elec_angle = ((next_encoder_lap_angle * POLE_PAIRS) + ANGLE_BASE).normalized(); 

    }
};

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

    Cell<iq16> current_ref;
    // std::atomic<iq16> current_ref2;
    auto motor_ctl_callback = [&]{
        if(can.pending() != 0) return;
        std::vector<hal::CanClassicFrame> frames;

        frames.push_back(req_msg::SetLowQuadMotorSetpoint{
            .setpoints = {
                CurrentCode::from_amps(current_ref.get()).unwrap(), 
                // CurrentCode::from_amps(current_ref).unwrap(), 
                // CurrentCode::from_amps(current_ref).unwrap(), 
                // CurrentCode::from_amps(current_ref).unwrap(), 
                0,0,0
            }
        }.to_can_frame());

        for(const auto & frame : frames){
            can.write(frame).examine();
        }
        // DEBUG_PRINTLN(CurrentCode::from_amps(current_ref).unwrap().to_amps());
    };

    timer.init({
        .count_freq = hal::NearestFreq(500), 
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
        current_ref.set(1.0_iq16 * iq16(sinpu(ctime)));
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
                            DEBUG_PRINTLN(
                                // resp.position.to_angle().cast_inner<uq16>().to_turns(), 
                                resp.position.to_angle().to_turns(), 
                                resp.speed.to_rpm(),
                                // resp.position.bits,
                                CurrentCode::from_amps(current_ref.get()).unwrap().to_amps(),
                                resp.current.to_amps(),
                                static_cast<uint8_t>(resp.loop_mode),
                                resp.exception_code.to_bits()
                            );
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

            if(0)DEBUG_PRINTLN_IDLE(
                // cnt_.load()
            );
        });
    }
}