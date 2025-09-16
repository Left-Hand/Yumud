#include "src/testbench/tb.h"
#include <atomic>


#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/polymorphism/traits.hpp"
#include "core/polymorphism/reflect.hpp"
#include "core/sync/timer.hpp"
#include "core/utils/sumtype.hpp"
#include "core/string/utils/strconv2.hpp"
#include "core/utils/combo_counter.hpp"
#include "core/utils/delayed_semphr.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/analog/opa/opa.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/IMU/Axis6/BMI160/BMI160.hpp"
#include "drivers/GateDriver/MP6540/mp6540.hpp"

#include "types/vectors/quat.hpp"


#include "robots/gesture/comp_est.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/cannet/can_chain.hpp"

#include "robots/commands/joint_commands.hpp"
#include "robots/commands/machine_commands.hpp"
#include "robots/commands/nmt_commands.hpp"
#include "robots/nodes/msg_factory.hpp"

#include "types/regions/perspective_rect.hpp"

#include "dsp/motor_ctrl/position_filter.hpp"
#include "dsp/motor_ctrl/calibrate_table.hpp"
#include "dsp/motor_ctrl/ctrl_law.hpp"
#include "dsp/motor_ctrl/elecrad_compsator.hpp"
#include "dsp/controller/pi_ctrl.hpp"
#include "dsp/controller/adrc/leso.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "digipw/prelude/abdq.hpp"



using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::digipw;
using namespace ymd::dsp;
using namespace ymd::robots;

using robots::NodeRole;
using robots::MsgFactory;


static constexpr auto SVPWM_MAX_VOLT = 3.87_r;

static constexpr size_t MACHINE_CTRL_FREQ = 200;
static constexpr auto DELTA_TIME_MS = 1000ms / MACHINE_CTRL_FREQ;
static constexpr auto DELTA_TIME = DELTA_TIME_MS.count() * 0.001_q20;

static constexpr uint32_t CHOPPER_FREQ = 25000;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;
static constexpr q20 PITCH_JOINT_POSITION_LIMIT = 0.2_r;
static constexpr size_t CANMSG_QUEUE_SIZE = 8;

static constexpr auto MAX_STATIC_SHOT_ERR = 0.007_q20; 
static constexpr auto PITCH_SEEKING_ANGLE = 0.012_q20;
static constexpr q20 PITCH_MAX_POSITION = 0.06_r;
static constexpr q20 PITCH_MIN_POSITION = -0.03_r;

static constexpr auto ADVANCED_BLINK_PERIOD_MS = 7000ms;
static constexpr auto STATIC_SHOT_FIRE_MS = 180ms;

static constexpr auto GEN2_TIMEOUT = 1700ms;
static constexpr auto GEN3_TIMEOUT = 3700ms;


using Vector2u8 = Vec2<uint8_t>;
using Vector2q20 = Vec2<q20>;



//CTAD


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


struct ErrPosition{
    // std::array<Vec2<uint8_t>, 4> points;
    bf16 px;
    bf16 py;
    bf16 z;
    bf16 e;

    friend OutputStream & operator << (OutputStream & os, const ErrPosition self){
        return os << os.brackets<'('>() 
            << self.px << os.splitter() 
            << self.py << os.splitter() 
            << self.z << os.splitter() 
            << self.e << os.splitter() 
        << os.brackets<')'>();
    }
};


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



enum class CommandKind:uint8_t{
    ResetNode,
    SetPosition,
    SetPositionWithFwdSpeed,
    SetSpeed,
    SetKpKd,
    // Deactivate,
    // Activate,
    // PerspectiveRectInfo,
    StartSeeking,
    StartTracking,
    StopTracking,
    DeltaPosition,
    ErrXY,
    FwdXY
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
DEF_QUICK_COMMAND_BIND(StartSeeking)
DEF_QUICK_COMMAND_BIND(StartTracking)
DEF_QUICK_COMMAND_BIND(StopTracking)
DEF_QUICK_COMMAND_BIND(DeltaPosition)
DEF_QUICK_COMMAND_BIND(ErrXY)
DEF_QUICK_COMMAND_BIND(FwdXY)

}


using commands::SetPositionWithFwdSpeed;


static constexpr bool is_ringback_msg(const hal::CanMsg & msg, const NodeRole self_node_role){
    const auto [role, cmd] = dump_role_and_cmd<CommandKind>(msg.stdid().unwrap());
    return role == self_node_role;
};


[[maybe_unused]] static void init_adc(hal::AdcPrimary & adc){

    using hal::AdcChannelNth;
    using hal::AdcSampleCycles;

    adc.init(
        {
            {AdcChannelNth::VREF, AdcSampleCycles::T28_5}
        },{
            {AdcChannelNth::CH5, AdcSampleCycles::T13_5},
            {AdcChannelNth::CH4, AdcSampleCycles::T13_5},
            {AdcChannelNth::CH1, AdcSampleCycles::T13_5},
            {AdcChannelNth::VREF, AdcSampleCycles::T7_5},
        }, {}
    );

    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1CC4);
    adc.enable_auto_inject(DISEN);
}

enum class BlinkPattern:uint8_t{
    RGB,
    RED,
    GREEN,
    BLUE
};

struct Task{
    // Generic,
    // Advanced

    struct Generic{
        Milliseconds start_tick;

        Milliseconds millis_since_started(){
            return clock::millis() - start_tick; 
        }

        static Generic from_default(){
            return Generic{
                .start_tick = clock::millis()
            };
        }
    };

    struct Advanced{
        Milliseconds start_tick;

        Milliseconds millis_since_started(){
            return clock::millis() - start_tick; 
        }

        static Advanced from_advanced(){
            return Advanced{
                .start_tick = clock::millis()
            };
        }
    };
};


void nuedc_2025e_main(){
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

    const auto UART_BAUD = [&] -> uint32_t{
        switch(get_node_role(chip_id_crc_).unwrap()){
            case NodeRole::YawJoint:
                return 576000;
            case NodeRole::PitchJoint:
                return 115200;
            default:
                __builtin_unreachable();
        }
    }();

    auto & DBG_UART = hal::uart2;

    DBG_UART.init({
        .baudrate = UART_BAUD
    });



    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);

    auto & spi = hal::spi1;
    auto & timer1 = hal::timer1;
    auto & can = hal::can1;
    auto & adc = hal::adc1;

    auto led_blue_gpio_ = hal::PC<13>();
    auto led_red_gpio_ = hal::PC<14>();
    auto led_green_gpio_ = hal::PC<15>();

    auto mp6540_en_gpio_ = hal::PB<15>();
    auto mp6540_nslp_gpio_ = hal::PB<14>();
    auto mp6540_nfault_gpio_ = hal::PA<7>();
    auto ma730_cs_gpio_ = hal::PA<0>();

    auto & pwm_u = timer1.oc<1>();
    auto & pwm_v = timer1.oc<2>();
    auto & pwm_w = timer1.oc<3>(); 

    led_red_gpio_.outpp(); 
    led_blue_gpio_.outpp(); 
    led_green_gpio_.outpp();
    


    const auto self_node_role_ = get_node_role(chip_id_crc_)
        .expect(chip_id_crc_);

    const auto node_is_master_ = [&] -> bool{
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

    can.filter(0) 
        .apply(hal::CanFilterConfig::from_pair(
            hal::CanStdIdMaskPair::from_id_and_mask(
                comb_role_and_cmd(self_node_role_, uint8_t(0x00)), 
                hal::CanStdId(0b1111'000'0000), hal::CanRtrSpecfier::Discard
            ))
        )
    ;

    spi.init({
        .baudrate = 18_MHz
    });


    timer1.init({
        .freq = CHOPPER_FREQ, 
        .mode = hal::TimerCountMode::CenterAlignedUpTrig
    }, EN);

    timer1.oc<4>().init({.install_en = DISEN});
    timer1.oc<4>().cvr() = timer1.arr() - 1;

    pwm_u.init({});
    pwm_v.init({});
    pwm_w.init({});

    led_red_gpio_.outpp(); 
    led_blue_gpio_.outpp(); 
    led_green_gpio_.outpp();
    mp6540_nfault_gpio_.inana();

    can.init({
        .baudrate = hal::CanBaudrate::_1M,
        .mode = hal::CanMode::Normal
    });


    MA730 ma730_{
        &spi,
        spi.allocate_cs_gpio(&ma730_cs_gpio_)
            .unwrap()
    };

    BMI160 bmi160_{
        &spi,
        spi.allocate_cs_gpio(&ma730_cs_gpio_)
            .unwrap()
    };

    if(self_node_role_ == NodeRole::YawJoint){
        bmi160_.init({
            .acc_odr = BMI160::AccOdr::_200Hz,
            .acc_fs = BMI160::AccFs::_16G,
            .gyr_odr = BMI160::GyrOdr::_200Hz,
            .gyr_fs = BMI160::GyrFs::_500deg

            // .acc_odr = BMI160::AccOdr::_400Hz,
            // .acc_fs = BMI160::AccFs::_16G,
            // .gyr_odr = BMI160::GyrOdr::_400Hz,
            // .gyr_fs = BMI160::GyrFs::_500deg
        }).examine();
    }

    ma730_.init({
        .direction = CW
    }).examine();



    static constexpr auto mp6540_adc_scaler = MP6540::make_adc_scaler(10'000);

    [[maybe_unused]] auto u_sense = hal::ScaledAnalogInput(adc.inj<1>(), mp6540_adc_scaler);
    [[maybe_unused]] auto v_sense = hal::ScaledAnalogInput(adc.inj<2>(), mp6540_adc_scaler);
    [[maybe_unused]] auto w_sense = hal::ScaledAnalogInput(adc.inj<3>(), mp6540_adc_scaler);
    
    auto uvw_pwmgen = UvwPwmgen(&pwm_u, &pwm_v, &pwm_w);

    init_adc(adc);

    hal::PA<7>().inana();

    mp6540_en_gpio_.outpp(HIGH);
    mp6540_nslp_gpio_.outpp(HIGH);


    AlphaBetaCoord<q16> ab_volt_;
    
    dsp::PositionFilter pos_filter_{
        typename dsp::PositionFilter::Config{
            .fs = FOC_FREQ,
            .r = 105
        }
    };

    pos_filter_.set_base_lap_position(
        [&] -> q20{
            switch(self_node_role_){
                case NodeRole::YawJoint:
                    return {0.389_r + 0.5_r};
                case NodeRole::PitchJoint:
                    return {0.737_r};
                    // return {0.223_r};
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
                return PdCtrlLaw{.kp = 128.581_r, .kd = 18.7_r};
                // return PdCtrlLaw{.kp = 148.581_r, .kd = 14.7_r};
                // return PdCtrlLaw{.kp = 128.581_r, .kd = 12.7_r};
            case NodeRole::PitchJoint:
                return PdCtrlLaw{.kp = 36.281_r, .kd = 1.7_r};
                // return PdCtrlLaw{.kp = 39.281_r, .kd = 1.2_r};
                // return SqrtPdCtrlLaw{.kp = 166.281_r, .ks = 20.0_r, .kd = 30.4_r};
            default: 
                PANIC();
        }
    }();


    q20 q_volt_ = 0;
    Angle<q20> meas_elecrad_ = Angle<q20>::ZERO;

    q20 axis_target_position_ = 0;
    q20 axis_target_speed_ = 0;
    
    Microseconds exe_us_ = 0us;

    RunStatus run_status_;
    run_status_.state = RunState::Idle;

    auto update_sensors = [&]{ 
        ma730_.update().examine();

        if(self_node_role_ == NodeRole::YawJoint){
            bmi160_.update().examine();
        }

        const auto meas_lap_position = ma730_.read_lap_angle().examine(); 
        pos_filter_.update(meas_lap_position);
    };

    auto sensored_foc_cb = [&]{
        update_sensors();

        if(run_status_.state == RunState::Idle){

            SVPWM3::set_alpha_beta_dutycycle(uvw_pwmgen, AlphaBetaCoord<q16>::ZERO);
            leso_.reset();
            return;
        }

        const auto meas_lap_position = ma730_.read_lap_angle().examine(); 
        const auto meas_elecrad = elecrad_comp_(meas_lap_position);

        const auto meas_position = pos_filter_.position();
        const auto meas_speed = pos_filter_.speed();

        // [[maybe_unused]] static constexpr q20 omega = 2;
        // [[maybe_unused]] static constexpr q20 amp = 0.300_r;
        // [[maybe_unused]] const auto ctime = clock::time();

        const auto [axis_target_position, axis_target_speed] = ({
            std::make_tuple(
                // amp * sin(omega * ctime), amp * omega * cos(omega * ctime)
                axis_target_position_, axis_target_speed_
            );
        });


        [[maybe_unused]] const auto targ_position = axis_target_position;
        [[maybe_unused]] const auto targ_speed = axis_target_speed;


        #if 0
        const auto q_volt = 1.3_r;
        #else
        const auto q_volt = CLAMP2(
            pd_ctrl_law_(targ_position - meas_position, targ_speed - meas_speed)
        , SVPWM_MAX_VOLT);
        #endif

        [[maybe_unused]] const auto ab_volt = DqCoord<q16>{
            .d = 0, 
            .q = CLAMP2(q_volt - leso_.disturbance(), SVPWM_MAX_VOLT)
            // CLAMP2(q_volt, SVPWM_MAX_VOLT)
        }.to_alphabeta(meas_elecrad);


        static constexpr auto INV_BUS_VOLT = q16(1.0/12);

        SVPWM3::set_alpha_beta_dutycycle(
            uvw_pwmgen, 
            ab_volt * INV_BUS_VOLT
        );

        leso_.update(meas_speed, q_volt);

        q_volt_ = q_volt;
        meas_elecrad_ = meas_elecrad;
    };

    adc.attach(hal::AdcIT::JEOC, {0,0}, 
        [&]{
            const auto m = clock::micros();
            sensored_foc_cb();
            exe_us_ = clock::micros() - m;
        }, EN
    );

    
    bool seeked_before_tracking_ = false;
    bool report_en_ = false;
    
    Option<Milliseconds> may_advanced_start_tick_ = None;
    Option<Milliseconds> may_stk_start_tick_ = None;
    Option<Milliseconds> may_last_static_shot_tick_ = None;
    Vec2<q20> captured_err_xy_ = {0, 0};
    Option<ErrPosition> may_err_position_ = None;

    RingBuf<hal::CanMsg, CANMSG_QUEUE_SIZE> msg_queue_;

    auto write_can_msg = [&](const hal::CanMsg & msg){
        // if(msg.is_extended()) PANIC();
        if(msg.is_extended()) return;

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


    auto joint_start_seeking = [&]{
        may_err_position_ = None;
        run_status_.state = RunState::Seeking;
    };

    auto joint_start_tracking = [&]{
        run_status_.state = RunState::Tracking;
    };

    auto joint_to_idle = [&]{
        run_status_.state = RunState::Idle;
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

    auto publish_err_xy = [&](const commands::ErrXY errxy){
        publish_to_both_joints(errxy);
    };

    [[maybe_unused]] auto publish_fwd_xy = [&](const commands::FwdXY fwdxy){
        publish_to_both_joints(fwdxy);
    };
    

    auto publish_laser_en = [&](const bool on){

        const auto msg_id = [&]{
            static constexpr auto ON_ID = comb_role_and_cmd(NodeRole::Laser, LaserCommand::On);
            
            if(on) return ON_ID;
            else return comb_role_and_cmd(NodeRole::Laser, LaserCommand::Off);
            // if(on) return hal::CanStdId(0x183);
            // else return hal::CanStdId(0x184);
        }();

        const auto msg = hal::CanMsg::from_list(msg_id, {});
        write_can_msg(msg);
    };


    auto publish_dall = [&]{
        publish_joints_stop_tracking();
        publish_laser_en(false);
    };

    auto update_err_xy = [&](Vec2<q20> err_xy){
        // static constexpr real_t alpha_x = 0.8_r;
        // static constexpr real_t alpha_y = 0.5_r;
        // captured_err_xy_.x = captured_err_xy_.x * alpha_x + err_xy.x * (1 - alpha_x);
        // captured_err_xy_.y = captured_err_xy_.y * alpha_y + err_xy.y * (1 - alpha_y);

        captured_err_xy_.x = err_xy.x;
        captured_err_xy_.y = err_xy.y;
    };
    

    auto can_subscriber_service = [&]{
        [[maybe_unused]] auto delta_target_position_by_command = 
            [&](const commands::DeltaPosition & cmd){
            switch(self_node_role_){
                case NodeRole::YawJoint:{
                    const auto cmd_delta_position = q20(cmd.delta_position);
                    axis_target_speed_ = q20(cmd_delta_position * MACHINE_CTRL_FREQ);
                    axis_target_position_ = (pos_filter_.position() + cmd_delta_position);
                    break;
                }
                case NodeRole::PitchJoint:{
                    const auto cmd_delta_position = q20(cmd.delta_position);
                    axis_target_speed_ = q20(cmd_delta_position * MACHINE_CTRL_FREQ);
                    axis_target_position_ = CLAMP(
                        axis_target_position_ + cmd_delta_position,
                        PITCH_MIN_POSITION, PITCH_MAX_POSITION
                    );
                    break;
                }
                default:
                    __builtin_unreachable();
            }
        };

        [[maybe_unused]] auto set_target_position_by_command = 
            [&](const commands::SetPosition & cmd){
            switch(self_node_role_){
                case NodeRole::YawJoint:{
                    axis_target_speed_ = 0;
                    axis_target_position_ = q20(cmd.position);
                    break;
                }
                case NodeRole::PitchJoint:{
                    axis_target_speed_ = 0;
                    axis_target_position_ = CLAMP(
                        q20(cmd.position),
                        PITCH_MIN_POSITION, PITCH_MAX_POSITION
                    );
                    break;
                }
                default:
                    __builtin_unreachable();
            }
        };

        auto dispatch_msg = [&](const CommandKind kind,const std::span<const uint8_t> payload){

            switch(kind){
            case CommandKind::ResetNode:
                sys::reset();
                break;

            case CommandKind::DeltaPosition:{
                const auto may_cmd = serde::make_deserialize<serde::RawBytes,
                    commands::DeltaPosition>(payload);
                if(may_cmd.is_ok()) delta_target_position_by_command(may_cmd.unwrap());
            }
                break;

            case CommandKind::SetPosition:{
                const auto may_cmd = serde::make_deserialize<serde::RawBytes,
                    commands::SetPosition>(payload);
                if(may_cmd.is_ok()) set_target_position_by_command(may_cmd.unwrap());
                break;
            }

            case CommandKind::StartSeeking:
                joint_start_seeking();
                break;

            case CommandKind::StartTracking:
                joint_start_tracking();
                break;

            case CommandKind::StopTracking:
                joint_to_idle();
                break;

            case CommandKind::ErrXY:{
                const auto may_cmd = serde::make_deserialize<serde::RawBytes,
                    commands::ErrXY>(payload);

                const auto cmd = may_cmd.unwrap();
                if(may_cmd.is_ok()) update_err_xy({q20(cmd.px), q20(cmd.py)});
                break;
            }

            case CommandKind::FwdXY:
                break;
            default:
                // PANIC("unknown command", std::bit_cast<uint8_t>(cmd));
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
            const auto & msg = may_msg.unwrap();
            process_msg(msg);
        }
    };

    [[maybe_unused]] auto publish_joint_position_with_fwd_speed = [&](
        const NodeRole node_role, 
        const commands::SetPositionWithFwdSpeed & cmd
    ){
        const auto msg = MsgFactory<CommandKind>{node_role}(cmd);
        write_can_msg(msg);
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

    auto blink_service = [&]{

        const auto blink_pattern = [&] -> BlinkPattern{

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

        switch(blink_pattern){
            case BlinkPattern::RED:
                led_red_gpio_ = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                led_blue_gpio_ = LOW;
                led_green_gpio_ = LOW;
                break;
            case BlinkPattern::BLUE:
                led_red_gpio_ = LOW;
                led_blue_gpio_ = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                led_green_gpio_ = LOW;
                break;
            case BlinkPattern::GREEN:
                led_red_gpio_ = LOW;
                led_blue_gpio_ = LOW;
                led_green_gpio_ = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                break;
            case BlinkPattern::RGB:
                led_red_gpio_ = BoolLevel::from((uint32_t(clock::millis().count()) % 200) > 100);
                led_blue_gpio_ = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                led_green_gpio_ = BoolLevel::from((uint32_t(clock::millis().count()) % 800) > 400);
                break;
        }
    };

    auto start_generic_task = [&](){
        //如果当前找到误差 说明已经对准 是基础2
        //如果当前找不到误差 说明没有对准 是基础3
        seeked_before_tracking_ = may_err_position_.is_none();

        publish_laser_en(false);

        if(seeked_before_tracking_){
            publish_joints_start_seeking();
        }else{
            publish_joints_start_tracking();
        }

        may_last_static_shot_tick_ = None;
        may_stk_start_tick_ = Some(clock::millis());
        may_advanced_start_tick_ = None;
    };

    auto start_advanced_task = [&](){
        seeked_before_tracking_ = false;
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

            rpc::make_property_with_limit("kp", &pd_ctrl_law_.kp, 0, 240),

            rpc::make_property_with_limit("kd", &pd_ctrl_law_.kd, 0, 30),

            rpc::make_function("setp", [&](real_t p){axis_target_position_ = p;}),
            rpc::make_function("errp", [&](
                const real_t px, 
                const real_t py,
                const real_t z, 
                const real_t e
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

            rpc::make_function("mp", [&](){
                DEBUG_PRINTLN(axis_target_position_, pos_filter_.position());
            }),

            rpc::make_function("stk", [&](){ 
                start_generic_task();
            }),

            rpc::make_function("lsr", [&](const bool on){
                publish_laser_en(on);
            }),

            rpc::make_function("rpen", [&](){
                report_en_ = true;
            }),

            rpc::make_function("rpdis", [&](){
                report_en_ = false;
            }),

            rpc::make_function("fn2", [&](){
                start_advanced_task();
            }),

            rpc::make_function("fn3", [&](){
                start_advanced_task();
            })
        );

        repl_server.invoke(list);
    };

    [[maybe_unused]] auto on_gimbal_idle = [&]{
        publish_laser_en(false);
    };

    [[maybe_unused]] auto on_gimbal_seeking = [&]{
        if(may_err_position_.is_some()){
            publish_joints_start_tracking();
            run_status_.state = RunState::Tracking;
        }
    };

    [[maybe_unused]] auto on_gimbal_tracking = [&]{ 
        if(may_advanced_start_tick_.is_some()){
            const auto delta_ms = clock::millis() - may_advanced_start_tick_.unwrap();
            if(delta_ms < ADVANCED_BLINK_PERIOD_MS){
                publish_laser_en(uint32_t(delta_ms.count()) % 100 <= 6);
            }else{
                publish_laser_en(true);
            }
            return;
        }

        const bool shot_en = [&] -> bool{
            if(may_last_static_shot_tick_.is_some()) return false;


        
            if(may_stk_start_tick_.is_some()){
                if(seeked_before_tracking_){
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
            may_last_static_shot_tick_ = Some(clock::millis());
            publish_laser_en(true);
        };

        if(shot_en){
            shot();
        }

        if(may_last_static_shot_tick_.is_some() and 
            clock::millis() - may_last_static_shot_tick_.unwrap() > STATIC_SHOT_FIRE_MS
        ){
            publish_laser_en(false);
        }
    };

    [[maybe_unused]] auto on_joint_seeking_ctl = [&]{ 
        //pitch 抬高到指定角度
        //yaw 波浪运动
        switch(self_node_role_){
            case NodeRole::PitchJoint:
                publish_joint_position(NodeRole::PitchJoint, 
                    //10度
                    commands::SetPosition{
                        // .position = 0.015_r
                        .position = PITCH_SEEKING_ANGLE
                    });
                break;
            case NodeRole::YawJoint:{
                const auto ctime = clock::time();
                auto hesitate_spin_curve = [&](const real_t t){
                    return (- 0.3_q20 * (1.25_r + sinpu(3.0_r * t)) / MACHINE_CTRL_FREQ);
                };

                publish_joint_delta_position(NodeRole::YawJoint, 
                    commands::DeltaPosition{
                        .delta_position = hesitate_spin_curve(ctime)
                    });
                break;
            } 
            
            default:
                PANIC();
        }
    };

    [[maybe_unused]] auto on_joint_tracking_ctl = [&]{
        //pitch P控制
        //yaw P控制

        static constexpr auto YAW_KP = 1.85_q20;
        static constexpr auto PITCH_KP = 0.65_q20;

        switch(self_node_role_){
            case NodeRole::PitchJoint:{
                const auto kp_contri = PITCH_KP * q20(-captured_err_xy_.y);
                // if(std::abs(captured_err_xy_.y) < 0.1_r) kp_contri = kp_contri * 1.5_q20; 

                publish_joint_delta_position(NodeRole::PitchJoint, 
                    commands::DeltaPosition{
                        .delta_position = kp_contri / MACHINE_CTRL_FREQ
                    }
                );
                break;
            }

            case NodeRole::YawJoint:{
                static constexpr auto THIS_BMI160_YAW_GYR_BIAS = 0.0020_q24;

                // imu补偿
                const auto yaw_gyr = -(
                    bmi160_.read_gyr().examine().z + 
                    THIS_BMI160_YAW_GYR_BIAS)
                ;

                const auto kp_contri = CLAMP2(YAW_KP * q20(-captured_err_xy_.x), 0.12_q20);
                publish_joint_delta_position(NodeRole::YawJoint, 
                    commands::DeltaPosition{
                        .delta_position = kp_contri / MACHINE_CTRL_FREQ + 
                            q20(yaw_gyr) * DELTA_TIME * q20(-1.0 / TAU * 1.00)
                    });
                break;
            }
            default:
                PANIC();
        }

    };

    auto master_service = [&]{
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

        if(may_err_position_.is_some()){
            const auto err_position = may_err_position_.unwrap();

            publish_err_xy(commands::ErrXY{
                .px = err_position.px, 
                .py = err_position.py
            });

            // publish_fwd_xy(commands::ErrXY{
            //     .px = err_position.px, 
            //     .py = err_position.py
            // });
        }else{

            //找不到目标时停留在原地
            publish_err_xy(commands::ErrXY{
                .px = 0, 
                .py = 0
            });
        }
    };

    auto joint_service = [&]{
        switch(run_status_.state){
            case RunState::Seeking:
                on_joint_seeking_ctl();
                break;
            case RunState::Tracking:
                on_joint_tracking_ctl();
                break;
            default:
                break;
        }
    };

    while(true){
        static auto timer = async::RepeatTimer::from_duration(1000ms / MACHINE_CTRL_FREQ);
        timer.invoke_if([&]{
            if(node_is_master_){
                master_service();
            }
            can_subscriber_service();
            joint_service();
            can_subscriber_service();
            blink_service();
        });

        repl_service();

        can_subscriber_service();

        if(report_en_){
            static auto report_timer = async::RepeatTimer::from_duration(DELTA_TIME_MS);
            
            report_timer.invoke_if([&]{

                DEBUG_PRINTLN_IDLE(
                    // pos_filter_.cont_position(), 
                    // pos_filter_.speed(),
                    // meas_elecrad_,
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