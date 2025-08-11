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
#include "core/utils/combo_counter.hpp"
#include "core/utils/delayed_semphr.hpp"

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


#include "robots/gesture/comp_est.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/cannet/can_chain.hpp"

#include "robots/commands/joint_commands.hpp"
#include "robots/commands/machine_commands.hpp"
#include "robots/commands/nmt_commands.hpp"
#include "robots/nodes/msg_factory.hpp"

#include "types/regions/perspective_rect/perspective_rect.hpp"

#include "dsp/motor_ctrl/position_filter.hpp"
#include "dsp/motor_ctrl/calibrate_table.hpp"
#include "dsp/motor_ctrl/ctrl_law.hpp"
#include "dsp/motor_ctrl/elecrad_compsator.hpp"
#include "dsp/controller/pi_ctrl.hpp"
#include "dsp/controller/adrc/leso.hpp"

#include "digipw/SVPWM/svpwm.hpp"
#include "digipw/SVPWM/svpwm3.hpp"


using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::digipw;
using namespace ymd::dsp;
using namespace ymd::robots;

using robots::NodeRole;
using robots::MsgFactory;

static constexpr uint32_t CHOPPER_FREQ = 25000;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;
static constexpr q20 PITCH_JOINT_POSITION_LIMIT = 0.2_r;
static constexpr size_t CANMSG_QUEUE_SIZE = 8;
static constexpr auto A4_WIDTH = 0.210_r;
static constexpr auto A4_HEIGHT = 0.297_r; 
static constexpr auto ADVANCED_PREPARE_TIME = 15000ms;  

static constexpr size_t GIMBAL_CTRL_FREQ = 200;

[[maybe_unused]] static constexpr auto DELTA_TIME_MS = 5ms;
[[maybe_unused]] static constexpr auto DELTA_TIME = DELTA_TIME_MS.count() * 0.001_q20;
[[maybe_unused]] static constexpr size_t FREQ = 1000ms / DELTA_TIME_MS;



#define DEF_COMMAND_BIND(K, T) \
template<> \
struct command_to_kind<CommandKind, T>{ \
    static constexpr CommandKind KIND = K; \
};\
template<> \
struct kind_to_command<CommandKind, K>{ \
    using type = T; \
};


#define DEF_QUICK_COMMAND_BIND(NAME) \
    DEF_COMMAND_BIND(CommandKind::NAME, commands::NAME)




using Vector2u8 = Vector2<uint8_t>;
using Vector2q20 = Vector2<q20>;



static constexpr Vector2<q20> a4_coord_to_uv_coord(const Vector2<q20> a4_coord){
    constexpr auto INV_A4_WIDTH = 1 / A4_WIDTH;
    constexpr auto INV_A4_HEIGHT = 1 / A4_HEIGHT;    
    return {a4_coord.x * INV_A4_WIDTH, a4_coord.y * INV_A4_HEIGHT};
}

static constexpr Vector2<q20> uv_coord_to_a4_coord(const Vector2<q20> uv_coord){
    return {uv_coord.x * A4_WIDTH, uv_coord.y * A4_HEIGHT};
}

static constexpr auto A4_CENTER_COORD = uv_coord_to_a4_coord(Vector2<q20>(0.5_r, 0.5_r));

// 主函数
static constexpr Option<std::array<Vector2u8, 4>> defmt_u8x4(std::string_view str) {
    if (str.size() != 16) return None;


    // 将两个 16 进制字符转换为 u8 (constexpr)
    auto parse_u8_hex_pair = [](char c1, char c2) -> Option<uint8_t> {
            // 将单个 16 进制字符转换为数值 (constexpr)
        auto hex_char_to_u8 = [](char c) -> uint8_t{
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
            if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
            return 0xFF; // 无效字符标记
        };

        uint8_t high = hex_char_to_u8(c1);
        uint8_t low = hex_char_to_u8(c2);
        if (high == 0xFF || low == 0xFF) return None;
        return Some(static_cast<uint8_t>((high << 4) | low));
    };

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



enum class CommandKind:uint8_t{
    ResetNode,
    SetPosition,
    SetPositionWithFwdSpeed,
    SetSpeed,
    SetKpKd,
    // Deactivate,
    // Activate,
    PerspectiveRectInfo,
    StartSeeking,
    StartTracking,
    StopTracking,
    DeltaPosition,
    ErrPosition
};

enum class LaserCommand:uint8_t{
    On = 0x33,
    Off
};

static constexpr q20 PITCH_MAX_POSITION = 0.06_r;
static constexpr q20 PITCH_MIN_POSITION = -0.06_r;


namespace commands{ 
    using namespace robots::joint_commands;
    using namespace robots::nmt_commands;
}


DEF_QUICK_COMMAND_BIND(ResetNode)
DEF_QUICK_COMMAND_BIND(SetPosition)
DEF_QUICK_COMMAND_BIND(SetPositionWithFwdSpeed)
DEF_QUICK_COMMAND_BIND(SetSpeed)
DEF_QUICK_COMMAND_BIND(SetKpKd)
DEF_QUICK_COMMAND_BIND(PerspectiveRectInfo)
DEF_QUICK_COMMAND_BIND(StartSeeking)
DEF_QUICK_COMMAND_BIND(StartTracking)
DEF_QUICK_COMMAND_BIND(StopTracking)
DEF_QUICK_COMMAND_BIND(DeltaPosition)
DEF_QUICK_COMMAND_BIND(ErrPosition)

}


using commands::SetPositionWithFwdSpeed;


static constexpr bool is_ringback_msg(const hal::CanMsg & msg, const NodeRole self_node_role){
    const auto [role, cmd] = dump_role_and_cmd<CommandKind>(msg.stdid().unwrap());
    return role == self_node_role;
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
    RED,
    GREEN,
    BLUE
};

class CanSinkIntf{
public:
    virtual void write(const hal::CanMsg & msg) = 0;
};

class MyCanSink:public CanSinkIntf{
public:
    explicit MyCanSink(hal::Can & can):
        can_(can){;}

    void write(const hal::CanMsg & msg) override{
        can_.write(msg).examine();
    }
private:
    hal::Can & can_;
};

class LaserEndpoint{
public:
    LaserEndpoint(CanSinkIntf & sink):
        sink_(sink){;}
    void set_en(const bool on){

        const auto msg_id = [&]{
            // if(on) return comb_role_and_cmd(NodeRole::Laser, LaserCommand::On);
            // else return comb_role_and_cmd(NodeRole::Laser, LaserCommand::Off);
            if(on) return hal::CanStdId(0x183);
            else return hal::CanStdId(0x184);
        }();

        const auto msg = hal::CanMsg::from_list(msg_id, {});
        sink_.write(msg);
    };
private:
    CanSinkIntf & sink_;
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
    DEBUGGER.no_brackets();

    auto & spi = hal::spi1;
    auto & timer = hal::timer1;
    auto & can = hal::can1;
    auto & adc = hal::adc1;

    auto & ledb = hal::PC<13>();
    auto & ledr = hal::PC<14>();
    auto & ledg = hal::PC<15>();

    auto & en_gpio = hal::PB<15>();
    auto & nslp_gpio = hal::PB<14>();

    auto & pwm_u = timer.oc<1>();
    auto & pwm_v = timer.oc<2>();
    auto & pwm_w = timer.oc<3>(); 

    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    


    const auto self_node_role_ = get_node_role(chip_id_crc_)
        .expect(chip_id_crc_);

    const bool chip_is_master_ = [&] -> bool{
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

    MyCanSink can_sink_(hal::can1);
    LaserEndpoint laser_{can_sink_};

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
            .acc_odr = BMI160::AccOdr::_400Hz,
            .acc_fs = BMI160::AccFs::_16G,
            .gyr_odr = BMI160::GyrOdr::_400Hz,
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

    en_gpio.set();
    nslp_gpio.set();


    AbVoltage ab_volt_;
    
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
                // return PdCtrlLaw{.kp = 126.581_r, .kd = 2.4_r};
                // return SqrtPdCtrlLaw{.kp = 96.581_r, .kd = 2.4_r};
            case NodeRole::PitchJoint:
                return PdCtrlLaw{.kp = 26.281_r, .kd = 1.4_r};
                // return SqrtPdCtrlLaw{.kp = 166.281_r, .ks = 20.0_r, .kd = 30.4_r};
            default: 
                PANIC();
        }
    }();


    q20 q_volt_ = 0;
    q20 meas_elecrad_ = 0;

    q20 axis_target_position_ = 0;
    q20 axis_target_speed_ = 0;
    
    Microseconds exe_us_ = 0us;

    RunStatus run_status_;
    run_status_.state = RunState::Idle;

    
    [[maybe_unused]] auto sensored_foc_cb = [&]{
        ma730_.update().examine();
        if(self_node_role_ == NodeRole::YawJoint){
            bmi160_.update().examine();
        }

        const auto meas_lap_position = ma730_.read_lap_position().examine(); 
        pos_filter_.update(meas_lap_position);


        const q20 meas_elecrad = elecrad_comp_(meas_lap_position);

        const auto meas_position = pos_filter_.position();
        const auto meas_speed = pos_filter_.speed();
        [[maybe_unused]] const auto ctime = clock::time();

        const auto [axis_target_position, axis_target_speed] = ({
            std::make_tuple(
                axis_target_position_, axis_target_speed_
            );
        });


        [[maybe_unused]] const auto targ_position = axis_target_position;
        [[maybe_unused]] const auto targ_speed = axis_target_speed;


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

        // [[maybe_unused]] const auto ctime = clock::time();
        #if 0
        {
            const auto [s,c] = sincospu(3.5_r * ctime);
            const auto amp = 2.3_r;
            svpwm_.set_ab_volt(c * amp, s * amp);
        }
        #else
        if(run_status_.state != RunState::Idle){
            svpwm_.set_ab_volt(ab_volt[0], ab_volt[1]);
        }else{
            svpwm_.set_ab_volt(0, 0);
        }
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


    bool is_still_mode_ = false;

    Option<real_t> position_when_stk_ = None;
    Option<Milliseconds> advanced_start_ms_ = None;
    Option<Milliseconds> start_fire_ms_ = None;




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



    auto publish_gimbal_start_seeking = [&]{
        publish_to_both_joints(commands::StartSeeking{});
    };

    auto publish_gimbal_start_tracking = [&]{
        publish_to_both_joints(commands::StartTracking{});
    };

    auto publish_err_position = [&](const commands::ErrPosition errpos){
        publish_to_both_joints(errpos);
    };
    
    auto publish_gimbal_stop_tracking = [&]{
        publish_to_both_joints(commands::StopTracking{});
    };

    // auto publish_gimbal_stop_updating = [&]{
    //     publish_to_both_joints(commands::StopUpdating{});
    // };

    bool report_en_ = false;
    
    Option<PerspectiveRect<q20>> may_a4_rect_ = None;
    auto gimbal_start_seeking = [&]{
        may_a4_rect_ = None;
        run_status_.state = RunState::Seeking;
    };

    auto gimbal_start_tracking = [&]{
        run_status_.state = RunState::Tracking;
    };

    auto gimbal_stop_tracking = [&]{
        run_status_.state = RunState::Idle;
    };

    auto publish_dall = [&]{
        publish_gimbal_stop_tracking();
        laser_.set_en(false);
    };


    auto on_a4_founded = [&](const PerspectiveRect<q20> rect){
        may_a4_rect_ = Some(rect);
    };

    auto on_a4_lost = [&]{
        may_a4_rect_ = None;
    };



    
    
    Vector2<q20> err_position_ = {0, 0};
    bool laser_is_oneshot_ = false;
    bool laser_onshot_ = true;

    auto update_err_position = [&](Vector2<q20> err_position){
        static constexpr real_t alpha_x = 0.8_r;
        static constexpr real_t alpha_y = 0.5_r;
        err_position_.x = err_position_.x * alpha_x + err_position.x * (1 - alpha_x);
        err_position_.y = err_position_.y * alpha_y + err_position.y * (1 - alpha_y);
    };

    auto set_err_position = [&](Vector2<q20> err_position){
        err_position_.x = err_position.x;
        err_position_.y = err_position.y;
    };

    auto can_subscriber_service = [&]{
        [[maybe_unused]] auto delta_target_by_command = 
            [&](const commands::DeltaPosition & cmd){
            switch(self_node_role_){
                case NodeRole::YawJoint:{
                    const auto cmd_delta_position = q20(cmd.delta_position);
                    axis_target_speed_ = q20(cmd_delta_position * GIMBAL_CTRL_FREQ);
                    // axis_target_position_ = (pos_filter_.position() + cmd_delta_position);
                    axis_target_position_ = (axis_target_position_ + cmd_delta_position);
                    break;
                }
                case NodeRole::PitchJoint:{
                    const auto cmd_delta_position = q20(cmd.delta_position);
                    axis_target_speed_ = q20(cmd_delta_position * GIMBAL_CTRL_FREQ);
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

        [[maybe_unused]] auto set_target_by_command = 
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

        auto dispatch_msg = [&](const CommandKind cmd,const std::span<const uint8_t> payload){

            switch(cmd){
            case CommandKind::ResetNode:
                sys::reset();
                break;
                

            case CommandKind::DeltaPosition:{
                const auto may_cmd = serde::make_deserialize<serde::RawBytes,
                    commands::DeltaPosition>(payload);
                if(may_cmd.is_ok()) delta_target_by_command(may_cmd.unwrap());
            }
                break;

            case CommandKind::SetPosition:{
                const auto may_cmd = serde::make_deserialize<serde::RawBytes,
                    commands::SetPosition>(payload);
                if(may_cmd.is_ok()) set_target_by_command(may_cmd.unwrap());
                break;
            }

            case CommandKind::PerspectiveRectInfo:{
                
                // may_a4_rect_ =       
                if(payload.size() != 8) break;          
                may_a4_rect_ = Some(PerspectiveRect<q20>::from_u8x8(
                    std::span<const uint8_t, 8>(payload.data(), payload.size())));
                break;
            }

            case CommandKind::StartSeeking:
                gimbal_start_seeking();
                break;

            case CommandKind::StartTracking:
                gimbal_start_tracking();
                break;

            case CommandKind::StopTracking:
                gimbal_stop_tracking();
                break;
            case CommandKind::ErrPosition:{
                const auto may_cmd = serde::make_deserialize<serde::RawBytes,
                    commands::ErrPosition>(payload);
                if(may_cmd.is_ok()) update_err_position(may_cmd.unwrap().to_vec2());
                break;
            }
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

    bool is_fn3_mode_ = false;

    [[maybe_unused]] auto report_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(5ms);
        
        timer.invoke_if([&]{
            const auto may_center = may_a4_rect_
                .map([&](const PerspectiveRect<q20> rect){
                    return rect.perspective_center();
                });


            DEBUG_PRINTLN(
                // pos_filter_.cont_position(), 
                // pos_filter_.speed(),
                // meas_elecrad_,
                // q_volt_,
                // pos_filter_.position(),
                // pos_filter_.speed(),
                // axis_target_position_,
                // axis_target_speed_,
                err_position_
                // sinpu(ctime) / GIMBAL_CTRL_FREQ * 0.2_r
                // may_a4_rect_
                // yaw_angle_,
                // pos_filter_.lap_position(),
                // run_status_.state
                // may_a4_rect_.unwrap().compute_homography(),
                // may_center
            );
        });
    };

    auto blink_service = [&]{
        static auto timer = async::RepeatTimer::from_duration(10ms);
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
            }
        });
    };

    [[maybe_unused]] auto on_gimbal_idle = [&]{
        laser_.set_en(false);
    };

    [[maybe_unused]] auto on_gimbal_seeking = [&]{
        if(may_a4_rect_.is_some()){
            publish_gimbal_start_tracking();
            run_status_.state = RunState::Tracking;
        }
    };

    [[maybe_unused]] auto on_gimbal_tracking = [&]{ 
        if(laser_is_oneshot_){
            // static Option<Milliseconds> last_ms_ = None;

            static DelayedSemphr semphr_ = DelayedSemphr(800ms);
            static Option<Milliseconds> last_shot_ms_ = None;

            // if(laser_onshot_ == true){
            if((ABS(err_position_.x) < 0.003_q20) and (ABS(err_position_.y) < 0.003_q20)){
                if(laser_onshot_ == true){
                    semphr_.give();
                }
            }

            const bool shot_en = [&]{
                if(semphr_.take() and laser_onshot_ == true) return true;
                if(start_fire_ms_.is_some()){
                    if(position_when_stk_.is_some() 
                        and (pos_filter_.position() - position_when_stk_.unwrap()) > 0.3_r 
                        and clock::millis() - start_fire_ms_.unwrap() > 3700ms){
                            start_fire_ms_ = None;
                            position_when_stk_ = None;
                            return true;
                        }

                    else if(clock::millis() - start_fire_ms_.unwrap() > 1700ms){
                        start_fire_ms_ = None;
                        return true;
                    }
                }
                return false;
            }();
            
            auto shot = [&]{
                if(last_shot_ms_.is_some()) return;
                last_shot_ms_ = Some(clock::millis());
                laser_.set_en(true);
            };

            if(shot_en){
                shot();
            }

            if(last_shot_ms_.is_some() and clock::millis() - last_shot_ms_.unwrap() > 200ms){
                laser_.set_en(false);
                last_shot_ms_ = None;
            }


        }else{
            laser_.set_en(true);
        }
    };

    [[maybe_unused]] auto gimbal_proc_err_position = [&]{
        if(may_a4_rect_.is_none()){
            publish_err_position(commands::ErrPosition{
                .x = 0, .y = 0});
        }

        publish_to_both_joints(commands::PerspectiveRectInfo{
            may_a4_rect_.unwrap().to_u8x8()
        });

        const auto expect_center_uv_coord = Vector2{105.2_q20 / 255, 95.0_q20 / 255};
        // const auto expect_center_uv_coord = Vector2{112.2_q20 / 255, 95.0_q20 / 255};
        const auto track_target_uv_coord = may_a4_rect_.unwrap()
            .perspective_center();

        const auto ctime = clock::time();
        const auto route_play_ratio = q24(frac(ctime * 0.1_r));
        const auto route_play_coord = Vector2{0.0006_q20, 0_q20}
            .rotated(route_play_ratio * real_t(TAU));
            
        auto expect_uv_coord = expect_center_uv_coord;
        if(is_fn3_mode_)expect_uv_coord += route_play_coord;
        
        const auto err_position = (track_target_uv_coord - expect_uv_coord);

        publish_err_position(commands::ErrPosition{
            .x = err_position.x, .y = err_position.y});

    };


    [[maybe_unused]] auto on_joint_seeking_ctl = [&]{ 

        switch(self_node_role_){
            case NodeRole::PitchJoint:
                publish_joint_position(NodeRole::PitchJoint, 
                    //10度
                    commands::SetPosition{
                        .position = 0.02_r
                    });
                break;
            case NodeRole::YawJoint:{
                const auto ctime = clock::time();
                if(may_a4_rect_.is_some()) break;
                publish_joint_delta_position(NodeRole::YawJoint, 
                    commands::DeltaPosition{
                        // .delta_position = (- 0.3_q20 * (1.35_r + sin ( 6 * real_t(PI) * ctime)) / GIMBAL_CTRL_FREQ)
                        .delta_position = (- 0.3_q20 * (1.35_r + sin ( 6 * real_t(PI) * ctime)) / GIMBAL_CTRL_FREQ)
                        // .delta_position = 0
                    });
                break;
            } 
            
            default:
                PANIC();
        }

    };


    [[maybe_unused]] auto on_joint_still_tracking_ctl = [&]{
        static constexpr auto YAW_DELTA = 2.85_q20 / GIMBAL_CTRL_FREQ;
        static constexpr auto PITCH_DELTA = 0.75_q20 / GIMBAL_CTRL_FREQ;

        publish_joint_delta_position(NodeRole::PitchJoint, 
            commands::DeltaPosition{
                .delta_position = -q20(err_position_.y) * PITCH_DELTA
            }
        );

        publish_joint_delta_position(NodeRole::YawJoint, 
            commands::DeltaPosition{
                .delta_position = YAW_DELTA * q20(-err_position_.x)
            });
    };

    [[maybe_unused]] auto on_joint_dyna_tracking_ctl = [&]{
        static constexpr auto YAW_DELTA = 1.85_q20 / GIMBAL_CTRL_FREQ;
        static constexpr auto PITCH_DELTA = 0.55_q20 / GIMBAL_CTRL_FREQ;
        

        if(advanced_start_ms_.is_some()){
            if(clock::millis() - advanced_start_ms_.unwrap() < ADVANCED_PREPARE_TIME){
                laser_.set_en(false);
            }else{
                laser_.set_en(true);
                advanced_start_ms_ = None;
            }
        }

        switch(self_node_role_){
            case NodeRole::PitchJoint:
                publish_joint_delta_position(NodeRole::PitchJoint, 
                    commands::DeltaPosition{
                        .delta_position = -q24(err_position_.y) * PITCH_DELTA
                    });
                break;
            case NodeRole::YawJoint:{

                static constexpr auto THIS_BMI160_YAW_GYR = 0.0020_q24;
                //imu补偿
                const auto yaw_gyr = -(
                    bmi160_.read_gyr().examine().z + 
                    THIS_BMI160_YAW_GYR)
                ;

                // static q24 inte = 0;
                // static constexpr q24 INTE_MAX = 0.02_r;
                // static constexpr q24 KI = 0.015_q20 / GIMBAL_CTRL_FREQ;

                // inte = CLAMP2(inte + KI * (-err_position_.x), INTE_MAX);

                publish_joint_delta_position(NodeRole::YawJoint, 
                    commands::DeltaPosition{
                        .delta_position = YAW_DELTA * q20(-err_position_.x) + 
                            q20(yaw_gyr) * DELTA_TIME * q20(-1.0 / TAU * 1.03)
                    });
                break;
            }
            
            default:
                PANIC();
        }
    };

    [[maybe_unused]] auto gimbal_sm_service = [&]{ 
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

    auto on_stk_command = [&]{
        position_when_stk_ = Some(pos_filter_.position());
        publish_gimbal_start_seeking();
        is_still_mode_ = true;
        laser_is_oneshot_ = true;
        laser_onshot_ = true;
        laser_.set_en(false);
        start_fire_ms_ = Some(clock::millis());
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

            rpc::make_function("a4c", [&](StringView str){ 
                auto may_rect = defmt_u8x4(str);
                if(may_rect.is_none()) return;
                on_a4_founded(PerspectiveRect<q20>::from_u8points(may_rect.unwrap()));
            }),

            rpc::make_function("a4n", [&](){ 
                on_a4_lost();
            }),

            // rpc::make_function("mp", [&](){
            //     DEBUG_PRINTLN(axis_target_position_, pos_filter_.position());
            // }),

            rpc::make_function("stk", [&](){ 
                on_stk_command();
            }),

            rpc::make_function("exy", [&](real_t x, real_t y){ 
                set_err_position({x,y});
            }),

            rpc::make_function("stp", [&](){ 
                publish_gimbal_stop_tracking();
            }),

            rpc::make_function("lsr", [&](const bool on){
                laser_.set_en(on);
            }),

            rpc::make_function("rpen", [&](){
                report_en_ = true;
            }),

            rpc::make_function("rpdis", [&](){
                report_en_ = false;
            }),

            rpc::make_function("dt", [&](const real_t dt){
                publish_joint_delta_position(self_node_role_, 
                commands::DeltaPosition{
                    .delta_position = dt
                });
            }),

            rpc::make_function("fn2", [&](){
                is_fn3_mode_ = false;
                is_still_mode_ = false;
                publish_gimbal_start_tracking();
                laser_is_oneshot_ = false;
                laser_onshot_ = false;
                advanced_start_ms_ = Some(clock::millis());
            }),

            rpc::make_function("fn3", [&](){
                is_fn3_mode_ = false;
                is_still_mode_ = false;
                publish_gimbal_start_tracking();
                laser_is_oneshot_ = false;
                laser_onshot_ = false;
                advanced_start_ms_ = Some(clock::millis());
            })
        );

        repl_server.invoke(list);
    };

    while(true){
        repl_service();

        can_subscriber_service();

        blink_service();

        if(chip_is_master_){
            gimbal_sm_service();
            gimbal_proc_err_position();
        }

        switch(run_status_.state){
            case RunState::Seeking:
                on_joint_seeking_ctl();
                break;
            case RunState::Tracking:
                if(is_still_mode_){
                    on_joint_still_tracking_ctl();
                }else{
                    on_joint_dyna_tracking_ctl();
                }
                break;
            default:
                break;
        }

        if(report_en_){
            report_service();
        }
    }
}
