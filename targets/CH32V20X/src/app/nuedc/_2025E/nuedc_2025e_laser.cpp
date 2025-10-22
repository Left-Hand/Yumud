#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/string/utils/streamed_string_splitter.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/analog/opa/opa.hpp"

#include "core/math/realmath.hpp"
#include "core/sync/timer.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/nodes/msg_factory.hpp"
#include "robots/nodes/node_role.hpp"

#include "types/vectors/vector2.hpp"

#include "nuedc_2025e_common.hpp"


using namespace ymd;
using namespace ymd::robots;
using namespace ymd::nuedc_2025e;


#define DBG_UART hal::uart2
static constexpr uint32_t PWM_FREQ = 10000;
static constexpr uint32_t ISR_FREQ = PWM_FREQ / 2;



static constexpr q20 PITCH_JOINT_POSITION_LIMIT = 0.2_r;


static constexpr auto ADVANCED_BLINK_PERIOD_MS = 7000ms;
static constexpr auto STATIC_SHOT_FIRE_MS = 180ms;

static constexpr auto GEN2_TIMEOUT = 1700ms;
static constexpr auto GEN3_TIMEOUT = 3700ms;


struct GenericTask{
    Milliseconds start_tick;

    Milliseconds millis_since_started(){
        return clock::millis() - start_tick; 
    }

    static GenericTask from_default(){
        return GenericTask{
            .start_tick = clock::millis()
        };
    }
};

struct AdvancedTask{
    Milliseconds start_tick;

    Milliseconds millis_since_started(){
        return clock::millis() - start_tick; 
    }

    static AdvancedTask from_advanced(){
        return AdvancedTask{
            .start_tick = clock::millis()
        };
    }
};

struct Task{
    // GenericTask,
    // AdvancedTask


};



void nuedc_2025e_laser_main(){
    DBG_UART.init({
        .baudrate = 576000
    });
    
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);
    DEBUG_PRINTLN("powerup");

    [[maybe_unused]] auto mode1_gpio   = hal::PB<1>();
    [[maybe_unused]] auto phase_gpio   = hal::PA<7>();

    phase_gpio.outpp(LOW);

    hal::timer3.init(
        {
            .freq = PWM_FREQ, 
            .count_mode = hal::TimerCountMode::CenterAlignedUpTrig
        },  
        EN
    );

    auto & laser_pwm = hal::timer3.oc<1>();
    laser_pwm.init({});

    auto led = hal::PB<8>();
    led.outpp(HIGH);


    auto & can = hal::can1;

    can.init({
        .remap = CAN1_REMAP,
        .mode = hal::CanMode::Normal,
        .timming_coeffs = hal::CanBaudrate(hal::CanBaudrate::_1M).to_coeffs()
    });


    can.filters<0>().apply(
        hal::CanFilterConfig::from_accept_all()
    );


    bool is_basic3 = false;
    bool report_en_ = false;
    
    Option<Milliseconds> may_advanced_start_tick_ = None;
    Option<Milliseconds> may_stk_start_tick_ = None;
    Option<Milliseconds> may_last_shot_tick_ = None;
    Option<ErrPosition> may_err_position_ = None;
    RunStatus run_status_;
    run_status_.state = RunState::Idle;

    auto set_laser_dutycycle = [&](real_t duty){
        DEBUG_PRINTLN("duty", duty);
        duty = CLAMP2(duty, 0.99_r);
        laser_pwm.set_dutycycle(ABS(duty));
    };


    auto write_can_msg = [&](const hal::CanMsg & msg){
        if(msg.is_extended()) PANIC();
        can.write(msg).examine();
    };



    auto publish_to_both_joints = [&]<typename T>
    (const T & cmd){
        {
            const auto msg = MsgFactory<CommandKind>{NodeRole::PitchJoint}(cmd);
            write_can_msg(msg);
        }
        {
            const auto msg = MsgFactory<CommandKind>{NodeRole::YawJoint}(cmd);
            write_can_msg(msg);
        }
    };


    auto publish_joints_start_seeking = [&]{
        publish_to_both_joints(commands::StartSeeking{});
    };

    auto publish_joints_start_tracking = [&]{
        publish_to_both_joints(commands::StartTracking{});
    };

    auto publish_joints_stop_tracking = [&]{
        publish_to_both_joints(commands::StopTracking{});
    };

    auto publish_dall = [&]{
        publish_joints_stop_tracking();
        set_laser_dutycycle(0);
    };


    [[maybe_unused]] auto blink_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(200ms);
        timer.invoke_if([&]{
            static BoolLevel last_state = LOW;
            last_state = ~last_state;
            led.write(last_state);
        });
    };


    auto handle_start_generic_task = [&](){
        //如果当前找到误差 说明已经对准 是基础2
        //如果当前找不到误差 说明没有对准 是基础3
        is_basic3 = may_err_position_.is_none();

        set_laser_dutycycle(0);

        if(is_basic3){
            publish_joints_start_seeking();
        }else{
            publish_joints_start_tracking();
        }

        may_last_shot_tick_ = None;
        may_stk_start_tick_ = Some(clock::millis());
        may_advanced_start_tick_ = None;
    };

    auto handle_start_advanced_task = [&](){
        is_basic3 = false;
        publish_joints_start_tracking();

        may_stk_start_tick_ = None;
        may_advanced_start_tick_ = Some(clock::millis());
    };

    [[maybe_unused]] auto repl_service = [&]{
        static robots::ReplServer repl_server{&DBG_UART, &DBG_UART};

        static const auto list = rpc::make_list(
            "list",
            rpc::make_function("rst", [](){sys::reset();}),
            rpc::make_function("outen", [&](){repl_server.set_outen(EN);}),
            rpc::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),

            rpc::make_function("dall", [&](){
                publish_dall();
            }),
            rpc::make_function("dty", [&](const real_t duty){set_laser_dutycycle(duty);}),
            rpc::make_function("errp", [&](
                const q16 px, 
                const q16 py,
                const q16 z, 
                const q16 e
            ){
                may_err_position_ = Some(ErrPosition{
                    .px = px,
                    .py = py,
                    .z = z,
                    .e = e
                });
            }),

            rpc::make_function("errn", [&](){ 
                may_err_position_ = None;
            }),

            rpc::make_function("stk", [&](){ 
                handle_start_generic_task();
            }),

            rpc::make_function("rpen", [&](){
                report_en_ = true;
            }),

            rpc::make_function("rpdis", [&](){
                report_en_ = false;
            }),

            rpc::make_function("fn2", [&](){
                handle_start_advanced_task();
            }),

            rpc::make_function("fn3", [&](){
                handle_start_advanced_task();
            })
        );

        repl_server.invoke(list);
    };

    [[maybe_unused]] auto publish_joint_delta_position = [&](
        const NodeRole node_role, 
        const commands::DeltaPosition & cmd
    ){
        const auto msg = MsgFactory<CommandKind>{node_role}(cmd);
        write_can_msg(msg);
    };

    [[maybe_unused]] auto publish_joint_position = [&](
        const NodeRole node_role, 
        const commands::SetPosition & cmd
    ){
        const auto msg = MsgFactory<CommandKind>{node_role}(cmd);
        write_can_msg(msg);
    };

    [[maybe_unused]] auto handle_gimbal_idle = [&]{
        set_laser_dutycycle(false);
    };

    [[maybe_unused]] auto handle_gimbal_seeking = [&]{
        if(may_err_position_.is_none()) return;
        publish_joints_start_tracking();
        run_status_.state = RunState::Tracking;
    };

    [[maybe_unused]] auto handle_gimbal_tracking = [&]{ 
        if(may_advanced_start_tick_.is_some()){
            const auto delta_ms = clock::millis() - may_advanced_start_tick_.unwrap();
            if(delta_ms < ADVANCED_BLINK_PERIOD_MS){
                set_laser_dutycycle(uint32_t(delta_ms.count()) % 100 <= 6);
            }else{
                set_laser_dutycycle(1);
            }
            return;
        }

        const bool shot_en = [&] -> bool{
            if(may_last_shot_tick_.is_some()) return false;
        
            if(may_stk_start_tick_.is_some()){
                if(is_basic3){
                    if(clock::millis() - may_stk_start_tick_.unwrap() > GEN3_TIMEOUT){
                        may_stk_start_tick_ = None;
                        return true;
                    }
                }else{
                    if(clock::millis() - may_stk_start_tick_.unwrap() > GEN2_TIMEOUT){
                        may_stk_start_tick_ = None;
                        return true;
                    }
                }
            }

            return false;
        }();
        
        auto shot = [&]{
            may_last_shot_tick_ = Some(clock::millis());
            set_laser_dutycycle(1);
        };

        if(shot_en){
            shot();
        }

        if(may_last_shot_tick_.is_some() and 
            clock::millis() - may_last_shot_tick_.unwrap() > STATIC_SHOT_FIRE_MS
        ){
            set_laser_dutycycle(false);
        }
    };

    [[maybe_unused]] auto joint_seeking_ctl = [&](){ 
        //pitch 抬高到指定角度
        //yaw 波浪运动
        {
            const auto command = commands::SetPosition{
                // .position = 0.015_r
                .position = PITCH_SEEKING_ANGLE
            };

            publish_joint_position(NodeRole::PitchJoint, command);
        }

        {
            const auto ctime = clock::time();
            auto hesitate_spin_curve = [&](const q16 t){
                return (- 0.3_q20 * (1.25_r + sinpu(3.0_r * t)) / MACHINE_CTRL_FREQ);
            };

            const auto command = commands::DeltaPosition{
                .delta_position = hesitate_spin_curve(ctime)
            };

            publish_joint_delta_position(NodeRole::YawJoint, command);
        } 
    };

    
    [[maybe_unused]] auto joint_tracking_ctl = [&](const Vec2<q20> err){
        //pitch P控制
        //yaw P控制

        static constexpr auto YAW_KP = 1.85_q20;
        static constexpr auto PITCH_KP = 0.65_q20;

        {
            const auto kp_contri = PITCH_KP * q20(-err.y);

            const auto command = commands::DeltaPosition{
                .delta_position = kp_contri / MACHINE_CTRL_FREQ
            };

            publish_joint_delta_position(NodeRole::PitchJoint, command);
        }

        {
            const auto kp_contri = CLAMP2(YAW_KP * q20(-err.x), 0.12_q20);
            const auto command = commands::DeltaPosition{
                .delta_position = kp_contri / MACHINE_CTRL_FREQ
            };

            publish_joint_delta_position(NodeRole::YawJoint, command);
        }
    };


    while(true){
        auto joint_service = [&]{
            switch(run_status_.state){
            case RunState::Seeking:
                joint_seeking_ctl();
                break;
            case RunState::Tracking:{
                auto e_info = may_err_position_.unwrap();
                joint_tracking_ctl(Vec2{q20(e_info.px), q20(e_info.py)});
                break;
            }
            
            default:
                break;
            }
        };

        auto master_service = [&]{
            switch(run_status_.state){
            case RunState::Idle: 
                handle_gimbal_idle();
                break;
            case RunState::Seeking: 
                handle_gimbal_seeking();
                break;
            case RunState::Tracking: 
                handle_gimbal_tracking();
                break;
            }
        };

        static auto timer = async::RepeatTimer::from_duration(1000ms / MACHINE_CTRL_FREQ);

        timer.invoke_if([&]{
            master_service();
            joint_service();
        });

        repl_service();
    
        if(report_en_){
            static auto report_timer = async::RepeatTimer::from_duration(DELTA_TIME_MS);
            
            report_timer.invoke_if([&]{

                DEBUG_PRINTLN_IDLE(
                    // pos_filter_.cont_position(), 
                    // pos_filter_.speed(),
                    // meas_elecangle_,
                    // q_volt_,
                    // pos_filter_.position(),
                    // pos_filter_.speed(),
                    // axis_target_position_,
                    // axis_target_speed_,
                    may_err_position_
                    // sinpu(ctime) / MACHINE_CTRL_FREQ * 0.2_r
                    // yaw_angle_,
                    // pos_filter_.lap_position(),
                    // run_status_.state
                    // may_center
                );
            });
        }


    }
}
