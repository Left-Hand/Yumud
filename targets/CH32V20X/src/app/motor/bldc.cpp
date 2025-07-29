#include "src/testbench/tb.h"
#include "utils.hpp"
#include <atomic>


#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/polymorphism/traits.hpp"
#include "core/polymorphism/reflect.hpp"
#include "core/sync/timer.hpp"
#include "core/utils/sumtype.hpp"
#include "core/string/utils/strconv2.hpp"


#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/IMU/Axis6/BMI160/BMI160.hpp"
#include "drivers/GateDriver/MP6540/mp6540.hpp"

#include "types/vectors/quat/Quat.hpp"

#include "digipw/SVPWM/svpwm.hpp"
#include "digipw/SVPWM/svpwm3.hpp"


#include "dsp/motor_ctrl/position_filter.hpp"
#include "dsp/motor_ctrl/calibrate_table.hpp"
#include "dsp/motor_ctrl/ctrl_law.hpp"
#include "dsp/observer/smo/SmoObserver.hpp"
#include "dsp/observer/lbg/RolbgObserver.hpp"
#include "dsp/observer/nonlinear/NonlinearObserver.hpp"
#include "dsp/controller/pi_ctrl.hpp"
#include "dsp/controller/adrc/leso.hpp"
#include "robots/gesture/comp_est.hpp"

#include "CurrentSensor.hpp"

#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/cannet/can_chain.hpp"

#include "robots/commands/joint_commands.hpp"
#include "robots/commands/machine_commands.hpp"
#include "robots/commands/nmt_commands.hpp"
#include "robots/nodes/msg_factory.hpp"

using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::foc;
using namespace ymd::digipw;
using namespace ymd::dsp;
using namespace ymd::robots;

using robots::NodeRole;
using robots::MsgFactory;

static constexpr uint32_t CHOPPER_FREQ = 25000;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;
static constexpr real_t JOINT_POSITION_LIMIT = 0.2_r;
static constexpr size_t CANMSG_QUEUE_SIZE = 8;


#define DEF_COMMAND_BIND(K, T) \
template<> \
struct command_to_kind<CommandKind, T>{ \
    static constexpr CommandKind KIND = K; \
};\
template<> \
struct kind_to_command<CommandKind, K>{ \
    using type = T; \
};


#define DEF_QUICK_COMMAND_BIND(NAME) DEF_COMMAND_BIND(CommandKind::NAME, commands::NAME)



namespace ymd{

struct ElecradCompensator{
    q16 base;
    uint32_t pole_pairs;

    constexpr q16 operator ()(const q16 lap_position) const {
        return (frac(frac(lap_position + base) * pole_pairs) * real_t(TAU));
    }
};


enum class CommandKind:uint8_t{
    ResetNode,
    SetPosition,
    SetPositionWithFwdSpeed,
    SetSpeed,
    SetKpKd
};

namespace commands{ 
    using namespace robots::joint_commands;
    using namespace robots::nmt_commands;
}


DEF_QUICK_COMMAND_BIND(ResetNode)
DEF_QUICK_COMMAND_BIND(SetPosition)
DEF_QUICK_COMMAND_BIND(SetPositionWithFwdSpeed)
DEF_QUICK_COMMAND_BIND(SetSpeed)
DEF_QUICK_COMMAND_BIND(SetKpKd)

}


struct TrackTarget{
    commands::SetPositionWithFwdSpeed roll ;
    commands::SetPositionWithFwdSpeed pitch;
};



static constexpr bool is_ringback_msg(const hal::CanMsg & msg, const NodeRole self_node_role){
    const auto [role, cmd] = dump_role_and_cmd<CommandKind>(msg.stdid().unwrap());
    return role == self_node_role;
};

struct Gesture{
    real_t roll;
    real_t pitch;

    constexpr Gesture lerp(const Gesture & other, const real_t ratio) const{
        return Gesture{
            .roll = roll + (other.roll - roll) * ratio,
            .pitch = pitch + (other.pitch - pitch) * ratio
        };
    }

    friend OutputStream & operator << (OutputStream & os, const Gesture & gesture){
        return os << "roll: " << gesture.roll << ", pitch: " << gesture.pitch;
    }
};


[[maybe_unused]] static void init_adc(hal::AdcPrimary & adc){

    using hal::AdcChannelIndex;
    using hal::AdcSampleCycles;

    adc.init(
        {
            {AdcChannelIndex::VREF, AdcSampleCycles::T28_5}
        },{
            {AdcChannelIndex::CH5, AdcSampleCycles::T13_5},
            {AdcChannelIndex::CH4, AdcSampleCycles::T13_5},
            {AdcChannelIndex::CH1, AdcSampleCycles::T13_5},
            {AdcChannelIndex::VREF, AdcSampleCycles::T7_5},
        }, {}
    );

    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1CC4);
    adc.enable_auto_inject(DISEN);
}

[[maybe_unused]]  static void init_opa(){
    hal::opa1.init<1,1,1>();
}



void bldc_main(){
    const auto chip_id_crc_ = sys::chip::get_chip_id_crc();
    auto get_node_role = [](const uint32_t chip_id_crc) -> Option<NodeRole>{
        switch(chip_id_crc){
            case 207097585:
                return Some(NodeRole::PitchJoint);
            case 736633164:
                return Some(NodeRole::RollJoint);
            case 1874498751:
                return Some(NodeRole::LeftWheel);
            case 0xF2A51D93:
                return Some(NodeRole::RightWheel);
            default:
                return None;
        }
    };

    const auto self_node_role_ = get_node_role(chip_id_crc_)
        .expect(chip_id_crc_);

    const auto node_is_master = [&] -> bool{
        switch(self_node_role_){
            case NodeRole::RollJoint:
                return true;
            default:
                return false;
        }
    }();

    static constexpr auto UART_BAUD = 576000;
    // my_can_ring_main();
    auto & DBG_UART = hal::uart2;

    auto & spi = hal::spi1;
    auto & timer = hal::timer1;
    auto & can = hal::can1;
    auto & adc = hal::adc1;

    auto & ledr = hal::portC[13];
    auto & ledb = hal::portC[14];
    auto & ledg = hal::portC[15];
    
    // auto & en_gpio = hal::portA[11];
    // auto & nslp_gpio = hal::portA[12];
    
    auto & en_gpio = hal::PB<15>();
    auto & nslp_gpio = hal::PB<14>();

    auto & pwm_u = timer.oc<1>();
    auto & pwm_v = timer.oc<2>();
    auto & pwm_w = timer.oc<3>(); 

    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    
    DBG_UART.init({
        .baudrate = UART_BAUD
    });


    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets();
    // DEBUGGER.force_sync(EN);
    // DEBUGGER.no_fieldname();
    // DEBUGGER.no_scoped();

    clock::delay(2ms);

    can.init({
        .baudrate = hal::CanBaudrate::_1M, 
        .mode = hal::CanMode::Normal
    });

    can[0].mask(
        {
            .id = hal::CanStdIdMask{0x200, hal::CanRemoteSpec::Any}, 
            .mask = hal::CanStdIdMask::from_ignore_low(7, hal::CanRemoteSpec::Any)
        },{
            .id = hal::CanStdIdMask{0x000, hal::CanRemoteSpec::Any}, 
            // .mask = hal::CanStdIdMask::from_ignore_low(7, hal::CanRemoteSpec::Any)
            .mask = hal::CanStdIdMask::from_accept_all()
        }
    );

    auto blink_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(10ms);
        timer.invoke_if([&]{
            ledr = BoolLevel::from((uint32_t(clock::millis().count()) % 200) > 100);
            ledb = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
            ledg = BoolLevel::from((uint32_t(clock::millis().count()) % 800) > 400);
        });
    };

    spi.init({18_MHz});

    en_gpio.outpp(HIGH);

    nslp_gpio.outpp(LOW);

    timer.init({
        .freq = CHOPPER_FREQ, 
        .mode = hal::TimerCountMode::CenterAlignedUpTrig
    });

    timer.oc<4>().init({.install_en = DISEN});
    timer.oc<4>().cvr() = timer.arr() - 1;

    pwm_u.init({});
    pwm_v.init({});
    pwm_w.init({});

    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    hal::portA[7].inana();

    can.init({
        .baudrate = hal::CanBaudrate::_1M,
        .mode = hal::CanMode::Normal
    });


    MA730 ma730_{
        &spi,
        spi.allocate_cs_gpio(&hal::portA[15])
            .unwrap()
    };


    ma730_.init({
        .direction = [&]{
            switch(self_node_role_){
                case NodeRole::LeftWheel:
                case NodeRole::RightWheel:
                    return CCW;
                default:
                    return CW;
            }
        }()
    }).examine();


    MP6540 mp6540{
        {pwm_u, pwm_v, pwm_w},
        {   
            adc.inj<1>(), 
            adc.inj<2>(), 
            adc.inj<3>()
        }
    };

    mp6540.init();
    mp6540.set_so_res(10'000);
    
    SVPWM3 svpwm_ {mp6540};
    

    [[maybe_unused]] auto & u_sense = mp6540.ch(1);
    [[maybe_unused]] auto & v_sense = mp6540.ch(2);
    [[maybe_unused]] auto & w_sense = mp6540.ch(3);
    

    // init_opa();
    init_adc(adc);

    hal::portA[7].inana();


    en_gpio.set();
    nslp_gpio.set();

    real_t self_blance_theta_ = 0;
    real_t self_blance_omega_ = 0;

    AbVoltage ab_volt_;
    
    dsp::PositionFilter pos_filter_{
        typename dsp::PositionFilter::Config{
            .fs = FOC_FREQ,
            .r = 85
        }
    };

    pos_filter_.set_base_lap_position(
        [&] -> real_t{
            switch(self_node_role_){
                case NodeRole::PitchJoint:
                    return {0.243_r};
                case NodeRole::RollJoint:
                    return {0.389_r + 0.5_r};
                case NodeRole::LeftWheel:
                    return {0.5_r};
                case NodeRole::RightWheel: 
                    return {0.5_r};
                default:
                    PANIC();
            }
        }()
    );

    ElecradCompensator elecrad_comp_{
        .base = [&]{
            switch(self_node_role_){
                case NodeRole::PitchJoint:
                    return -0.282_r;
                case NodeRole::RollJoint:
                    return -0.233_r;
                case NodeRole::LeftWheel:
                    return -0.5_r;
                case NodeRole::RightWheel: 
                    return -0.5_r;
                default:
                    PANIC();
            }
        }(),
        .pole_pairs = [&] -> uint32_t{
            switch(self_node_role_){
                case NodeRole::PitchJoint:
                    return 7;
                case NodeRole::RollJoint:
                    return 7;
                case NodeRole::LeftWheel:
                    return 7;
                case NodeRole::RightWheel: 
                    return 7;
                default:
                    PANIC();
            }
        }(),
    };

    dsp::Leso leso_{
        [&]{
            switch(self_node_role_){
                case NodeRole::PitchJoint:
                    return dsp::Leso::Config{
                        .b0 = 0.5_r,
                        .w = 1.2_r,
                        .fs = FOC_FREQ
                    };
                case NodeRole::RollJoint:
                    return dsp::Leso::Config{
                        .b0 = 1.3_r,
                        .w = 13,
                        .fs = FOC_FREQ
                    };
                case NodeRole::LeftWheel:
                case NodeRole::RightWheel:
                    return dsp::Leso::Config{
                        .b0 = 1.3_r,
                        .w = 13,
                        .fs = FOC_FREQ
                    };
                default:
                    PANIC();
            }
        }()
    };

    auto pd_ctrl_law_ = [&] -> PdCtrlLaw{ 
        switch(self_node_role_){
            case NodeRole::PitchJoint:
                // return PdCtrlLaw{.kp = 20.581_r, .kd = 0.78_r};
                // return PdCtrlLaw{.kp = 20.581_r, .kd = 1.00_r};
                return PdCtrlLaw{.kp = 36.281_r, .kd = 2.3_r};
                // return PdCtrlLaw{.kp = 12.581_r, .kd = 0.38_r};
            case NodeRole::RollJoint: 
                return PdCtrlLaw{.kp = 190.581_r, .kd = 30.38_r};
            case NodeRole::LeftWheel:
            case NodeRole::RightWheel:
                // return PdCtrlLaw{.kp = 20.581_r, .kd = 0.78_r};
                // return PdCtrlLaw{.kp = 20.581_r, .kd = 1.00_r};
                return PdCtrlLaw{.kp = 36.281_r, .kd = 2.3_r};
            default: 
                PANIC();
        }
    }();


    real_t q_volt_ = 0;
    real_t meas_elecrad_ = 0;

    real_t axis_target_position_ = 0;
    real_t axis_target_speed_ = 0;
    
    Microseconds exe_us_ = 0us;

    [[maybe_unused]] auto sensored_foc_cb = [&]{
        ma730_.update().examine();

        const auto meas_lap = 1 - ma730_.read_lap_position().examine(); 
        pos_filter_.update(meas_lap);


        const real_t meas_elecrad = elecrad_comp_(meas_lap);

        const auto meas_position = pos_filter_.position();
        const auto meas_speed = pos_filter_.speed();

        [[maybe_unused]] static constexpr real_t omega = 4;
        [[maybe_unused]] static constexpr real_t amp = 0.06_r;
        [[maybe_unused]] const auto ctime = clock::time();

        const auto [axis_target_position, axis_target_speed] = ({
            std::make_tuple(
                // amp * sin(omega * ctime), amp * omega * cos(omega * ctime)
                axis_target_position_, axis_target_speed_
            );
        });

        const auto [blance_position, blance_speed] = ({
            std::make_tuple(
                // self_blance_theta_ * real_t(-1/TAU), self_blance_omega_ * real_t(-1/TAU)
                self_blance_theta_ * real_t(-1/TAU), self_blance_omega_ * real_t(-1/TAU)
            );
        });

        [[maybe_unused]] const auto targ_position = blance_position + axis_target_position;
        [[maybe_unused]] const auto targ_speed = blance_speed + axis_target_speed;


        static constexpr auto MAX_VOLT = 2.7_r;

        #if 1
        const auto q_volt = 1.3_r;
        #else
        const auto q_volt = CLAMP2(
            pd_ctrl_law_(targ_position - meas_position, targ_speed - meas_speed)
        , MAX_VOLT);
        #endif

        [[maybe_unused]] const auto ab_volt = DqVoltage{
            0, 
            CLAMP2(q_volt - leso_.get_disturbance(), MAX_VOLT)
        }.to_alpha_beta(meas_elecrad);

        
        #if 1
        {
            const auto [s,c] = sincospu(3.5_r * ctime);
            const auto amp = 2.3_r;
            svpwm_.set_ab_volt(c * amp, s * amp);
        }
        #else
        svpwm_.set_ab_volt(ab_volt[0], ab_volt[1]);
        #endif

        leso_.update(meas_speed, q_volt);

        q_volt_ = q_volt;
        meas_elecrad_ = meas_elecrad;
    };

    adc.attach(hal::AdcIT::JEOC, {0,0}, 
        [&]{
            const auto m = clock::micros();
            sensored_foc_cb();
            exe_us_ = clock::micros() - m;
        }
    );


    RingBuf<hal::CanMsg, CANMSG_QUEUE_SIZE> msg_queue_;

    auto write_can_msg = [&](const hal::CanMsg & msg){
        if(msg.is_extended()) PANIC();

        const bool is_ringback = is_ringback_msg(msg, self_node_role_);

        if(is_ringback){
            msg_queue_.push(msg);
        }else{
            can.write(msg).examine();
        }
    };

    auto read_can_msg = [&] -> Option<hal::CanMsg>{
        while(can.available()){
            auto msg = can.read();
            if(msg.is_extended()) continue;
            if(not is_ringback_msg(msg, self_node_role_)) continue;
            msg_queue_.push(msg);
        }

        if(not msg_queue_.available())
            return None;

        return Some(msg_queue_.pop());
    };


    [[maybe_unused]] TrackTarget track_target_ = {
        .roll = {.position = 0, .speed = 0},
        .pitch = {.position = 0, .speed = 0}
    };

    auto update_joint_target = [&](const real_t p1, const real_t p2) -> void { 
        track_target_.roll.position   = CLAMP2(p1, JOINT_POSITION_LIMIT);
        track_target_.pitch.position  = CLAMP2(p2, JOINT_POSITION_LIMIT);
    };

    [[maybe_unused]] auto repl_service = [&]{
        static robots::ReplServer repl_server{&DBG_UART, &DBG_UART};

        static const auto list = rpc::make_list(
            "list",
            rpc::make_function("rst", [](){sys::reset();}),
            rpc::make_function("outen", [&](){repl_server.set_outen(EN);}),
            rpc::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),

            rpc::make_property_with_limit("kp", &pd_ctrl_law_.kp, 0, 240),

            rpc::make_property_with_limit("kd", &pd_ctrl_law_.kd, 0, 30),

            // rpc::make_function("vxy", [&](const real_t p1, const real_t p2){

            //     track_target_.roll.position   = CLAMP2(p1, JOINT_POSITION_LIMIT);
            //     track_target_.pitch.position  = CLAMP2(p2, JOINT_POSITION_LIMIT);
            // }),

            rpc::make_function("pxy", update_joint_target)
            
        );

        repl_server.invoke(list);
    };


    [[maybe_unused]] auto can_subscriber_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(5ms);
        [[maybe_unused]] auto set_target_by_command = [&](const commands::SetPositionWithFwdSpeed & cmd){
            axis_target_position_ = real_t(cmd.position);
            axis_target_speed_ = real_t(cmd.speed);
        };
        timer.invoke_if([&]{
            auto dispatch_msg = [&](const CommandKind cmd,const std::span<const uint8_t> payload){
                switch(cmd){
                case CommandKind::ResetNode:
                    sys::reset();
                    break;
                case CommandKind::SetPositionWithFwdSpeed:{
                    const auto cmd = serde::make_deserialize<serde::RawBytes,
                        commands::SetPositionWithFwdSpeed>(payload).examine();
                    set_target_by_command(cmd);
                }
                    break;

                default:
                    PANIC("unknown command", std::bit_cast<uint8_t>(cmd));
                    break;
                }
            };

            auto process_msg = [&](const hal::CanMsg & msg){
                const auto id = msg.stdid().unwrap();
                const auto [msg_role, msg_cmd] = dump_role_and_cmd<CommandKind>(id);
                if(msg_role != self_node_role_) return;
                dispatch_msg(msg_cmd, msg.iter_payload());
            };

            while(true){
                const auto may_msg = read_can_msg();
                if(may_msg.is_none()) break;
                process_msg(may_msg.unwrap());
            }
        });
    };


    [[maybe_unused]] auto can_publisher_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(5ms);

        auto publish_roll_joint_target = [&](const commands::SetPositionWithFwdSpeed & cmd){
            const auto msg = MsgFactory<CommandKind>{NodeRole::RollJoint}(cmd);

            write_can_msg(msg);
        };

        auto publish_pitch_joint_target = [&](const commands::SetPositionWithFwdSpeed & cmd){
            const auto msg = MsgFactory<CommandKind>{NodeRole::PitchJoint}(cmd);

            write_can_msg(msg);
        };

        timer.invoke_if([&]{

            if(node_is_master){
                publish_roll_joint_target(track_target_.roll);
                publish_pitch_joint_target(track_target_.pitch);
            }
        });
    };

    [[maybe_unused]] auto update_demo_track_service = [&]{ 
        static constexpr auto amp = 0.05_r;
        const auto ctime = clock::time();
        const auto [s, c] = sincos(ctime * 5);
        const auto p1 = c * amp;
        const auto p2 = s * amp;

        update_joint_target(p1, p2);
    };


    [[maybe_unused]] auto report_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(5ms);
        timer.invoke_if([&]{
            DEBUG_PRINTLN(pos_filter_.cont_position());
        });
    };


    while(true){
        repl_service();

        can_publisher_service();
        can_subscriber_service();

        blink_service();

        report_service();

    }
}