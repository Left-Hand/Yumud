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

#include "types/regions/perspective_rect/perspective_rect.hpp"

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
static constexpr real_t PITCH_JOINT_POSITION_LIMIT = 0.2_r;
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


// 将单个 16 进制字符转换为数值 (constexpr)
constexpr uint8_t hex_char_to_u8(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return 0xFF; // 无效字符标记
}

// 将两个 16 进制字符转换为 u8 (constexpr)
constexpr Option<uint8_t> parse_u8_hex_pair(char c1, char c2) {
    uint8_t high = hex_char_to_u8(c1);
    uint8_t low = hex_char_to_u8(c2);
    if (high == 0xFF || low == 0xFF) return None;
    return Some(static_cast<uint8_t>((high << 4) | low));
}


using Vector2u8 = Vector2<uint8_t>;
using Vector2q16 = Vector2<q16>;

// template<>
// struct command_to_kind<CommandKind, T>{
//     static constexpr CommandKind KIND = K;
// }
// template<>
// struct kind_to_command<CommandKind, K>{
//     using type = T;
// };
// DEF_COMMAND_BIND(LaserCommand, )
// static constexpr LaserCommand KIND = LaserCommand::On;

// struct LaserOff{
//     static constexpr LaserCommand KIND = LaserCommand::Off;
// };



// 主函数
static constexpr Option<std::array<Vector2u8, 4>> defmt_u8x4(std::string_view str) {
    if (str.size() != 16) return None;

    std::array<Vector2u8, 4> points{};

    for (size_t i = 0; i < 4; ++i) {
        size_t offset = i * 4;

        auto x = parse_u8_hex_pair(str[offset], str[offset + 1]);
        auto y = parse_u8_hex_pair(str[offset + 2], str[offset + 3]);

        if (!x.is_some() || !y.is_some()) {
            return None;
        }

        points[i] = Vector2u8{x.unwrap(), y.unwrap()};
    }

    return Some(points);
}

enum class RunState:uint8_t{
    Idle,
    Seeking,
    Tracking
};

DEF_DERIVE_DEBUG(RunState)
struct RunStatus{
    using State = RunState;
    RunState state = RunState::Idle;
};

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
    SetKpKd,
    Deactivate,
    Activate
};

enum class LaserCommand:uint8_t{
    On,
    Off
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
DEF_QUICK_COMMAND_BIND(Activate)
DEF_QUICK_COMMAND_BIND(Deactivate)

}


struct TrackTarget{
    commands::SetPositionWithFwdSpeed yaw ;
    commands::SetPositionWithFwdSpeed pitch;
};

using commands::SetPositionWithFwdSpeed;


static constexpr bool is_ringback_msg(const hal::CanMsg & msg, const NodeRole self_node_role){
    const auto [role, cmd] = dump_role_and_cmd<CommandKind>(msg.stdid().unwrap());
    return role == self_node_role;
};

struct Gesture{
    real_t yaw;
    real_t pitch;

    constexpr Gesture lerp(const Gesture & other, const real_t ratio) const{
        return Gesture{
            .yaw = yaw + (other.yaw - yaw) * ratio,
            .pitch = pitch + (other.pitch - pitch) * ratio
        };
    }

    friend OutputStream & operator << (OutputStream & os, const Gesture & gesture){
        return os << "yaw: " << gesture.yaw << ", pitch: " << gesture.pitch;
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


enum class BlinkPattern:uint8_t{
    RGB,
    RED,
    GREEN,
    BLUE
};


void bldc_main(){


    static constexpr auto UART_BAUD = 576000;
    // my_can_ring_main();
    auto & DBG_UART = hal::uart2;

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

    auto & spi = hal::spi1;
    auto & timer = hal::timer1;
    auto & can = hal::can1;
    auto & adc = hal::adc1;

    auto & ledb = hal::PC<13>();
    auto & ledr = hal::PC<14>();
    auto & ledg = hal::PC<15>();
    
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
    

    const auto chip_id_crc_ = sys::chip::get_chip_id_crc();

    auto get_node_role = [](const uint32_t chip_id_crc) -> Option<NodeRole>{
        switch(chip_id_crc){

            case 1874498751:
                return Some(NodeRole::YawJoint);
            case 0xF2A51D93:
                return Some(NodeRole::PitchJoint);
            case 0x110E3BFE:
                return Some(NodeRole::PitchJoint);
            default:
                return None;
        }
    };

    const auto self_node_role_ = get_node_role(chip_id_crc_)
        .expect(chip_id_crc_);

    const auto node_is_master = [&] -> bool{
        switch(self_node_role_){
            case NodeRole::YawJoint:
                return false;
            case NodeRole::PitchJoint:
                return true;
            default:
                PANIC();
        }
    }();


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

    
    BMI160 bmi160_{
        &spi,
        spi.allocate_cs_gpio(&hal::portA[0])
            .unwrap()
    };

    if(self_node_role_ == NodeRole::YawJoint){
        bmi160_.init({
            .acc_odr = BMI160::AccOdr::_200Hz,
            .acc_fs = BMI160::AccFs::_16G,
            .gyr_odr = BMI160::GyrOdr::_200Hz,
            .gyr_fs = BMI160::GyrFs::_500deg
        }).examine();
    }

    ma730_.init({
        .direction = CW
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
    bool motor_is_actived_ = false;
    // bool motor_is_actived_ = false;

    en_gpio.set();
    nslp_gpio.set();

    real_t self_blance_angle_ = 0;
    real_t self_blance_omega_ = 0;

    AbVoltage ab_volt_;
    
    dsp::PositionFilter pos_filter_{
        typename dsp::PositionFilter::Config{
            .fs = FOC_FREQ,
            .r = 105
        }
    };

    pos_filter_.set_base_lap_position(
        [&] -> real_t{
            switch(self_node_role_){
                case NodeRole::YawJoint:
                    return {0.389_r + 0.5_r};
                case NodeRole::PitchJoint:
                    // return {0.783_r};
                    return {0.023_r};
                default:
                    PANIC();
            }
        }()
    );

    ElecradCompensator elecrad_comp_{
        .base = [&]{
            switch(self_node_role_){
                case NodeRole::YawJoint:
                    return -0.211_r;
                case NodeRole::PitchJoint:
                    return -0.244_r;
                default:
                    PANIC();
            }
        }(),
        .pole_pairs = [&] -> uint32_t{
            return 7;
        }(),
    };

    dsp::Leso leso_{
        [&]{
            switch(self_node_role_){
                case NodeRole::YawJoint:
                    return dsp::Leso::Config{
                        .b0 = 0.5_r,
                        .w = 0.4_r,
                        .fs = FOC_FREQ
                    };
                case NodeRole::PitchJoint:
                    return dsp::Leso::Config{
                        .b0 = 1.4_r,
                        .w = 0.2_r,
                        .fs = FOC_FREQ
                    };

                default:
                    PANIC();
            }
        }()
    };

    auto pd_ctrl_law_ = [&]{ 
        switch(self_node_role_){
            case NodeRole::YawJoint: 
                return PdCtrlLaw{.kp = 78.581_r, .kd = 6.7_r};
                // return PdCtrlLaw{.kp = 126.581_r, .kd = 2.4_r};
                // return SqrtPdCtrlLaw{.kp = 96.581_r, .kd = 2.4_r};
            case NodeRole::PitchJoint:
                return PdCtrlLaw{.kp = 26.281_r, .kd = 1.4_r};
                // return SqrtPdCtrlLaw{.kp = 166.281_r, .ks = 20.0_r, .kd = 30.4_r};
            default: 
                PANIC();
        }
    }();


    real_t q_volt_ = 0;
    real_t meas_elecrad_ = 0;

    real_t axis_target_position_ = 0;
    real_t axis_target_speed_ = 0;
    
    Microseconds exe_us_ = 0us;
    q22 yaw_angle_ = 0;

    RunStatus run_status_;
    [[maybe_unused]] auto sensored_foc_cb = [&]{
        ma730_.update().examine();
        if(self_node_role_ == NodeRole::YawJoint){
            bmi160_.update().examine();
        }

        const auto meas_lap_position = ma730_.read_lap_position().examine(); 
        pos_filter_.update(meas_lap_position);


        const real_t meas_elecrad = elecrad_comp_(meas_lap_position);

        const auto meas_position = pos_filter_.position();
        const auto meas_speed = pos_filter_.speed();

        [[maybe_unused]] static constexpr real_t omega = 1;
        [[maybe_unused]] static constexpr real_t amp = 0.000_r;
        [[maybe_unused]] const auto ctime = clock::time();

        const auto [axis_target_position, axis_target_speed] = ({
            std::make_tuple(
                amp * sin(omega * ctime), amp * omega * cos(omega * ctime)
                // floor(4 * ctime) * 0.25_r, 0.0_r
                // axis_target_position_, axis_target_speed_
            );
        });

        const auto [blance_position, blance_speed] = ({
            std::make_tuple(
                // self_blance_angle_ * real_t(-1/TAU), self_blance_omega_ * real_t(-1/TAU)
                self_blance_angle_ * real_t(-1/TAU), self_blance_omega_ * real_t(-1/TAU)
            );
        });

        [[maybe_unused]] const auto targ_position = blance_position + axis_target_position;
        [[maybe_unused]] const auto targ_speed = blance_speed + axis_target_speed;


        static constexpr auto MAX_VOLT = 3.87_r;

        #if 0
        const auto q_volt = 1.3_r;
        #else
        const auto q_volt = CLAMP2(
            pd_ctrl_law_(targ_position - meas_position, targ_speed - meas_speed)
        , MAX_VOLT);
        #endif

        [[maybe_unused]] const auto ab_volt = DqVoltage{
            0, 
            CLAMP2(q_volt - leso_.get_disturbance(), MAX_VOLT)
            // CLAMP2(q_volt, MAX_VOLT)
        }.to_alpha_beta(meas_elecrad);

        
        #if 0
        {
            const auto [s,c] = sincospu(3.5_r * ctime);
            const auto amp = 2.3_r;
            svpwm_.set_ab_volt(c * amp, s * amp);
        }
        #else
        if(motor_is_actived_){
            svpwm_.set_ab_volt(ab_volt[0], ab_volt[1]);
        }else{
            svpwm_.set_ab_volt(0, 0);
        }
        // svpwm_.set_ab_volt(3.0_r, 0.0_r);
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
        .yaw = {.position = 0, .speed = 0},
        .pitch = {.position = 0, .speed = 0}
    };

    auto update_joint_target = [&](const real_t p1, const real_t p2) -> void { 
        track_target_.yaw.position   = CLAMP2(p1, PITCH_JOINT_POSITION_LIMIT);
        track_target_.pitch.position  = CLAMP2(p2, PITCH_JOINT_POSITION_LIMIT);
    };

    auto update_joint_target_with_speed = [&](
        const real_t p1, const real_t v1, 
        const real_t p2, const real_t v2
    ) -> void { 
        track_target_.yaw = SetPositionWithFwdSpeed{
            .position = p1,
            .speed = v1
        };

        track_target_.pitch = SetPositionWithFwdSpeed{
            .position = p2,
            .speed = v2
        };
    };

    auto set_motor_is_actived = [&](bool is_actived) -> void { 
        motor_is_actived_ = is_actived;
    };

    auto set_laser = [&](const bool on){

        const auto msg_id = [&]{
            // const auto factory = MsgFactory<LaserCommand>{NodeRole::Laser};
            // if(on) return factory(LaserOn{});
            // return factory(LaserOff{});
            if(on) return comb_role_and_cmd(NodeRole::Laser, LaserCommand::On);
            else return comb_role_and_cmd(NodeRole::Laser, LaserCommand::Off);
        }();

        write_can_msg(hal::CanMsg::from_list(msg_id, {}));
    };

    auto gimbal_start_seeking = [&]{
        run_status_.state = RunState::Seeking;
    };

    auto gimbal_stop_tracking = [&]{
        run_status_.state = RunState::Idle;
    };

    bool report_en_ = false;
    
    [[maybe_unused]] auto repl_service = [&]{
        static robots::ReplServer repl_server{&DBG_UART, &DBG_UART};

        static const auto list = rpc::make_list(
            "list",
            rpc::make_function("rst", [](){sys::reset();}),
            rpc::make_function("outen", [&](){repl_server.set_outen(EN);}),
            rpc::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),

            rpc::make_property_with_limit("kp", &pd_ctrl_law_.kp, 0, 240),

            rpc::make_property_with_limit("kd", &pd_ctrl_law_.kd, 0, 30),

            rpc::make_function("setp", update_joint_target),

            rpc::make_function("setps", update_joint_target_with_speed),
            rpc::make_function("a4c", [&](StringView str){ 
                DEBUG_PRINTLN("a4c", str, defmt_u8x4(str));
            }),

            rpc::make_function("act", [&](){ 
                set_motor_is_actived(true);
            }),

            rpc::make_function("deact", [&](){ 
                set_motor_is_actived(false);
            }),

            rpc::make_function("stk", [&](){ 
                gimbal_start_seeking();
            }),

            rpc::make_function("stp", [&](){ 
                gimbal_stop_tracking();
            }),

            rpc::make_function("lsr", [&](const bool on){
                set_laser(on);
            }),

            rpc::make_function("rpen", [&](){
                report_en_ = true;
            }),

            rpc::make_function("rpdis", [&](){
                report_en_ = false;
            })
        );

        repl_server.invoke(list);
    };

    auto can_subscriber_service = [&]{
        [[maybe_unused]] auto set_target_by_command = [&](const commands::SetPositionWithFwdSpeed & cmd){
            axis_target_position_ = real_t(cmd.position);
            axis_target_speed_ = real_t(cmd.speed);
        };
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

            case CommandKind::Activate:
                set_motor_is_actived(true);
                break;
            case CommandKind::Deactivate:
                set_motor_is_actived(false);
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
            // DEBUG_PRINTLN(msg);
            dispatch_msg(msg_cmd, msg.iter_payload());
        };

        while(true){
            const auto may_msg = read_can_msg();
            if(may_msg.is_none()) break;
            const auto & msg = may_msg.unwrap();
            process_msg(msg);
        }
    };


    [[maybe_unused]] auto can_publisher_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(5ms);

        auto publish_joint_position_with_fwd_speed = [&]<NodeRole Role>(const commands::SetPositionWithFwdSpeed & cmd){
            const auto msg = MsgFactory<CommandKind>{Role}(cmd);
            write_can_msg(msg);
        };

        timer.invoke_if([&]{
            if(node_is_master){
                publish_joint_position_with_fwd_speed.operator()<NodeRole::YawJoint>(track_target_.yaw);
                publish_joint_position_with_fwd_speed.operator()<NodeRole::PitchJoint>(track_target_.pitch);
            }
        });
    };

    [[maybe_unused]] auto update_demo_track_service = [&]{ 
        static constexpr auto amp = 0.005_r;
        const auto ctime = clock::time();
        const auto [s, c] = sincos(ctime * 5);
        const auto p1 = c * amp;
        const auto p2 = s * amp;

        update_joint_target(p1, p2);
    };


    [[maybe_unused]] auto report_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(5ms);
        timer.invoke_if([&]{
            DEBUG_PRINTLN(
                pos_filter_.cont_position(), 
                pos_filter_.speed(),
                meas_elecrad_,
                q_volt_,
                yaw_angle_,
                pos_filter_.lap_position()
            );
        });
    };

    [[maybe_unused]] auto yaw_selftrack_service = [&]{ 
        [[maybe_unused]] static constexpr auto DELTA_TIME_MS = 5ms;
        [[maybe_unused]] static constexpr auto DELTA_TIME = DELTA_TIME_MS.count() * 0.001_r;
        [[maybe_unused]] static constexpr size_t FREQ = 1000ms / DELTA_TIME_MS;
        static constexpr auto yaw_gyr_bias = 0.0020_q24;
        static auto timer = async::RepeatTimer::from_duration(DELTA_TIME_MS);
        timer.invoke_if([&]{
            const auto yaw_gyr = -(
                bmi160_.read_gyr().examine().z + 
                yaw_gyr_bias)
            ;
            yaw_angle_ += yaw_gyr * DELTA_TIME;

            self_blance_angle_ = yaw_angle_;
            self_blance_omega_ = yaw_gyr;
        });
    };

    [[maybe_unused]] auto on_gimbal_idle = [&]{

    };

    [[maybe_unused]] auto on_gimbal_seeking = [&]{
        // static auto timer = async::RepeatTimer::from_duration(100ms);
        // timer.invoke_if([&]{
        //     led_pin_.toggle();
        // });
    };

    [[maybe_unused]] auto on_gimbal_tracking = [&]{
        // static auto timer = async::RepeatTimer::from_duration(100ms);
        // timer.invoke_if([&]{
        //     led_pin_.toggle();
        // });
    };

    [[maybe_unused]] auto gimbal_service = [&]{ 
        switch(run_status_.state){
            case RunState::Idle: 
                on_gimbal_idle();
                break;
            case RunState::Seeking: 
                on_gimbal_seeking();
                break;
            case RunState::Tracking: 
                on_gimbal_tracking();
                break;
        }
    };

    auto blink_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(10ms);
        const auto blink_pattern = [&] -> BlinkPattern{
            if(not motor_is_actived_){
                return BlinkPattern::RED;
            }

            switch(run_status_.state){
                case RunState::Idle: 
                    return BlinkPattern::RED;
                case RunState::Seeking: 
                    return BlinkPattern::BLUE;
                case RunState::Tracking: 
                    return BlinkPattern::GREEN;
                default:
                    __builtin_unreachable();
            }
        }();

        timer.invoke_if([&]{
            switch(blink_pattern){
                case BlinkPattern::RED:
                    ledr = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                    ledb = LOW;
                    ledg = LOW;
                    break;
                case BlinkPattern::BLUE:
                    ledr = LOW;
                    ledb = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                    ledg = LOW;
                    break;
                case BlinkPattern::GREEN:
                    ledr = LOW;
                    ledb = LOW;
                    ledg = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                    break;
                case BlinkPattern::RGB:
                    ledr = BoolLevel::from((uint32_t(clock::millis().count()) % 200) > 100);
                    ledb = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                    ledg = BoolLevel::from((uint32_t(clock::millis().count()) % 800) > 400);
                    break;
            }
        });
    };

    while(true){
        repl_service();

        can_publisher_service();
        can_subscriber_service();

        blink_service();
        gimbal_service();

        if(report_en_){
            report_service();
        }

        update_demo_track_service();

        if(self_node_role_ == NodeRole::YawJoint){
            yaw_selftrack_service();
        }

    }
}