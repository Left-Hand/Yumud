#include "src/testbench/tb.h"
#include "app/stepper/ctrl.hpp"
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
#include "drivers/GateDriver/DRV8301/DRV8301.hpp"

#include "types/vectors/quat/Quat.hpp"

#include "digipw/SVPWM/svpwm.hpp"
#include "digipw/SVPWM/svpwm3.hpp"

#include "dsp/observer/smo/SmoObserver.hpp"
#include "dsp/observer/lbg/RolbgObserver.hpp"
#include "dsp/observer/nonlinear/NonlinearObserver.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "dsp/controller/pi_ctrl.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"


#include "CurrentSensor.hpp"

#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/cannet/can_chain.hpp"
#include "robots/commands/joint_commands.hpp"
#include "robots/commands/machine_commands.hpp"
#include "core/string/utils/streamed_string_splitter.hpp"
#include "core/container/inline_vector.hpp"
#include "core/utils/enum_dict.hpp"


using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::foc;
using namespace ymd::digipw;
using namespace ymd::dsp;
using namespace ymd::robots::joint_cmds;
using namespace ymd::robots::machine_cmds;

static constexpr uint32_t CHOPPER_FREQ = 25000;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;
static constexpr real_t JOINT_POSITION_LIMIT = 0.2_r;
static constexpr size_t CANMSG_QUEUE_SIZE = 8;

void init_adc(hal::AdcPrimary & adc){

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

__no_inline void init_opa(){
    hal::opa1.init<1,1,1>();
}



namespace ymd{

struct ElecradCompensator{
    q16 base;
    uint32_t pole_pairs;

    constexpr q16 operator ()(const q16 lap_position) const {
        return (frac(frac(lap_position + base) * pole_pairs) * real_t(TAU));
    }
};

struct PdCtrlLaw{
    real_t kp;
    real_t kd;

    constexpr real_t operator()(const real_t p_err, const real_t v_err) const {
        return kp * p_err + kd * v_err;
    } 
};


enum class NodeRole:uint8_t{
    Master = 0,
    RollJoint,
    PitchJoint,
    YawJoint,
    AxisX,
    AxisY,
    AxisZ,
    LeftWheel,
    RightWheel,
    Computer = 0x0f,
};


OutputStream & operator<<(OutputStream & os, const NodeRole & role){ 
    switch(role){
        case NodeRole::RollJoint: return os << "RollJoint";
        case NodeRole::PitchJoint: return os << "PitchJoint";
        default: __builtin_unreachable();
    }
}


enum class CommandKind:uint8_t{
    SetPosition,
    SetPositionAndSpeed,
    SetSpeed,
    SetKpKd
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


#define DEF_QUICK_COMMAND_BIND(NAME) DEF_COMMAND_BIND(CommandKind::NAME, robots::joint_cmds::NAME)

DEF_QUICK_COMMAND_BIND(SetPosition)
DEF_QUICK_COMMAND_BIND(SetPositionAndSpeed)
DEF_QUICK_COMMAND_BIND(SetSpeed)
DEF_QUICK_COMMAND_BIND(SetKpKd)

}


struct TrackTarget{
    robots::joint_cmds::SetPositionAndSpeed roll ;
    robots::joint_cmds::SetPositionAndSpeed pitch;
};

static constexpr auto dump_role_and_cmd(const hal::CanStdId id){
    const auto id_u11 = id.to_u11();
    return std::make_tuple(
        std::bit_cast<NodeRole>(uint8_t(id_u11 & 0x7f)),
        std::bit_cast<CommandKind>(uint8_t(id_u11 >> 7))
    );
};


static constexpr auto comb_role_and_cmd(const NodeRole role, const CommandKind cmd){
    const auto id_u11 = uint16_t(
        std::bit_cast<uint8_t>(role) 
        | (std::bit_cast<uint8_t>(cmd) << 7));
    return hal::CanStdId(id_u11);
};




struct MsgFactory{
    const NodeRole role;

    template<typename T>
    constexpr hal::CanMsg operator()(const T cmd){
        const auto id = comb_role_and_cmd(role, command_to_kind_v<CommandKind, T>);
        const auto iter = serde::make_serialize_iter<serde::RawBytes>(cmd);
        return hal::CanMsg::from_iter(id, iter).unwrap();
    };
};



static constexpr bool is_ringback(const hal::CanMsg & msg, const NodeRole self_node_role){
    const auto [role, cmd] = dump_role_and_cmd(msg.stdid().unwrap());
    return role == self_node_role;
};

struct GestureEstimator{
    struct Config{
        uint32_t fs;
    };

    constexpr explicit GestureEstimator(const Config & cfg):
        delta_time_(1_r / cfg.fs),
        comp_filter_(make_comp_filter_config(cfg.fs)){;}

    constexpr void process(const Vector3<q24> & acc,const Vector3<q24> & gyr){

        const auto len_acc = acc.length();
        const auto norm_acc = acc / len_acc;
        const auto base_roll_raw = atan2(norm_acc.x, norm_acc.y) + real_t(PI/2);
        const auto base_omega_raw = gyr.z;

        if(inited_ == false){
            theta_ = base_roll_raw;
            omega_ = base_omega_raw;
            inited_ = true;
            return;
        }

        // const auto base_roll = comp_filter(base_roll_raw, base_omega_raw);
        // DEBUG_PRINTLN_IDLE(
        //     // norm_acc.x, norm_acc.y,
        //     // base_roll_raw,
        //     // base_omega_raw,
        //     // base_roll,
        //     // pos_sensor_.position(),
        //     // pos_sensor_.lap_position(),
        //     // pos_sensor_.speed(),
        //     ma730.read_lap_position().examine(),
        //     meas_rad_
        //     // exe_us_
        //     // // leso.get_disturbance(),
        //     // meas_rad_
        // );

        const auto alpha_sqrt = (len_acc - 9.8_r) * 0.8_r;

        const auto alpha = MAX(1 - square(alpha_sqrt), 0) * 0.07_r;

        theta_ += (base_roll_raw - theta_) * alpha + (base_omega_raw * delta_time_) * (1 - alpha);
        omega_ = base_omega_raw;
    }

    auto theta_and_omega() const {
        return std::make_pair(theta_, omega_);
    }

private:
    using CompFilter = dsp::ComplementaryFilter<q20>;
    real_t delta_time_;
    CompFilter comp_filter_;
    real_t theta_ = 0;
    real_t omega_ = 0;
    bool inited_ = false;

    static constexpr typename CompFilter::Config
    make_comp_filter_config(const uint32_t fs){
        return {
            .kq = 0.90_r,
            .ko = 0.25_r,
            .fs = fs
        };
    }
};

void bldc_main(){
    // my_can_ring_main();
    auto & DBG_UART = hal::uart2;

    auto & spi = hal::spi1;
    auto & timer = hal::timer1;
    auto & can = hal::can1;
    auto & adc = hal::adc1;

    auto & ledr = hal::portC[13];
    auto & ledb = hal::portC[14];
    auto & ledg = hal::portC[15];
    auto & en_gpio = hal::portA[11];
    auto & nslp_gpio = hal::portA[12];

    auto & pwm_u = timer.oc<1>();
    auto & pwm_v = timer.oc<2>();
    auto & pwm_w = timer.oc<3>(); 

    DBG_UART.init({
        .baudrate = 576000
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
            .mask = hal::CanStdIdMask::IGNORE_LOW(7, hal::CanRemoteSpec::Any)
        },{
            .id = hal::CanStdIdMask{0x000, hal::CanRemoteSpec::Any}, 
            // .mask = hal::CanStdIdMask::IGNORE_LOW(7, hal::CanRemoteSpec::Any)
            .mask = hal::CanStdIdMask::ACCEPT_ALL()
        }
    );

    auto blink_service = [&]{
        static async::RepeatTimer timer{10ms};
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



    auto get_node_role = [] -> Option<NodeRole>{
        const auto chip_id_crc = sys::chip::get_chip_id_crc();
        switch(chip_id_crc){
            case 207097585:
                return Some(NodeRole::PitchJoint);
            case 736633164:
                return Some(NodeRole::RollJoint);
            default:
                return None;
        }
    };

    
    const auto self_node_role_ = get_node_role().examine();  



    MA730 ma730{
        &spi,
        spi.allocate_cs_gpio(&hal::portA[15])
            .unwrap()
    };

    BMI160 bmi{
        &spi,
        spi.allocate_cs_gpio(&hal::portA[0])
            .unwrap()
    };

    ma730.init({
        .direction = CW
    }).examine();

    // while(true){
    //     ma730.update().examine();
    //     DEBUG_PRINTLN(ma730.read_lap_position().examine());
    //     blink_service();
    //     clock::delay(5ms);
    // }

    
    bmi.init({
        .acc_odr = BMI160::AccOdr::_200Hz,
        .acc_fs = BMI160::AccFs::_16G,
        .gyr_odr = BMI160::GyrOdr::_200Hz,
        .gyr_fs = BMI160::GyrFs::_500deg
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
    
    SVPWM3 svpwm {mp6540};
    

    [[maybe_unused]] auto & u_sense = mp6540.ch(1);
    [[maybe_unused]] auto & v_sense = mp6540.ch(2);
    [[maybe_unused]] auto & w_sense = mp6540.ch(3);
    

    // init_opa();
    init_adc(adc);

    // CurrentSensor curr_sens = {u_sense, v_sense, w_sense};

    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    hal::portA[7].inana();


    en_gpio.set();
    nslp_gpio.set();

    real_t base_roll_theta_ = 0;
    real_t base_roll_omega_ = 0;

    AbVoltage ab_volt_;
    
    dsp::PositionSensor pos_sensor_{
        typename dsp::PositionSensor::Config{
            .r = 85,
            .fs = FOC_FREQ
        }
    };

    pos_sensor_.set_base_position(
        [&] -> real_t{
            switch(self_node_role_){
                case NodeRole::PitchJoint:
                    return {0.193_r};
                case NodeRole::RollJoint:
                    return {0.38_r + 0.5_r};
                default: __builtin_unreachable();
            }
        }()
    );

    ElecradCompensator elecrad_comp_{
        .base = [&]{
            switch(self_node_role_){
                case NodeRole::PitchJoint:
                    return -0.282_r;
                case NodeRole::RollJoint:
                    return -0.253_r;
                default: __builtin_unreachable();
            }
        }(),
        .pole_pairs = 7
    };


    real_t q_volt_ = 0;
    real_t meas_rad_ = 0;

    dsp::Leso leso{
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
                default: __builtin_unreachable();
            }
        }()
    };

    auto pd_ctrl_law_ = [&] -> PdCtrlLaw{ 
        switch(self_node_role_){
            case NodeRole::PitchJoint:
                // return PdCtrlLaw{.kp = 20.581_r, .kd = 0.78_r};
                // return PdCtrlLaw{.kp = 20.581_r, .kd = 1.00_r};
                return PdCtrlLaw{.kp = 24.281_r, .kd = 0.9_r};
                // return PdCtrlLaw{.kp = 12.581_r, .kd = 0.38_r};
            case NodeRole::RollJoint: 
                return PdCtrlLaw{.kp = 170.581_r, .kd = 25.38_r};
            default: __builtin_unreachable();
        }
    }();

    Microseconds exe_us_ = 0us;
    real_t track_pos_ = 0;
    real_t track_spd_ = 0;

    [[maybe_unused]] auto cb_sensored = [&]{
        ma730.update().examine();
        bmi.update().examine();

        const auto meas_lap = 1 - ma730.read_lap_position().examine(); 
        pos_sensor_.update(meas_lap);


        const real_t meas_rad = elecrad_comp_(meas_lap);

        const auto meas_pos = pos_sensor_.position();
        const auto meas_spd = pos_sensor_.speed();

        [[maybe_unused]] static constexpr real_t omega = 4;
        [[maybe_unused]] static constexpr real_t amp = 0.06_r;
        [[maybe_unused]] const auto clock_time = clock::time();

        const auto [track_pos, track_spd] = ({
            std::make_tuple(
                // amp * sin(omega * clock_time), amp * omega * cos(omega * clock_time)
                track_pos_, track_spd_
            );
        });

        const auto [blance_pos, blance_spd] = ({
            std::make_tuple(
                // base_roll_theta_ * real_t(-1/TAU), base_roll_omega_ * real_t(-1/TAU)
                base_roll_theta_ * real_t(-1/TAU), base_roll_omega_ * real_t(-1/TAU)
            );
        });

        const auto targ_pos = blance_pos + track_pos;
        const auto targ_spd = blance_spd + track_spd;


        static constexpr auto MAX_VOLT = 2.7_r;
        const auto q_volt = CLAMP2(
            pd_ctrl_law_(targ_pos - meas_pos, targ_spd - meas_spd)
        , MAX_VOLT);


        const auto ab_volt = DqVoltage{
            0, 
            CLAMP2(q_volt - leso.get_disturbance(), MAX_VOLT)
        }.to_ab(meas_rad);

        svpwm.set_ab_volt(ab_volt[0], ab_volt[1]);

        leso.update(meas_spd, q_volt);

        q_volt_ = q_volt;
        meas_rad_ = meas_rad;
    };

    adc.attach(hal::AdcIT::JEOC, {0,0}, 
        [&]{
            const auto m = clock::micros();
            cb_sensored();
            exe_us_ = clock::micros() - m;
        }
    );


    Fifo_t<hal::CanMsg, CANMSG_QUEUE_SIZE> msg_queue_;

    auto write_can_msg = [&](const hal::CanMsg & msg){
        if(msg.is_extended()) PANIC();

        const bool is_local = is_ringback(msg, self_node_role_);

        if(is_local){
            msg_queue_.push(msg);
        }else{
            can.write(msg);
        }
    };

    auto read_can_msg = [&] -> Option<hal::CanMsg>{
        while(can.available()){
            auto msg = can.read();
            if(msg.is_extended()) continue;
            if(not is_ringback(msg, self_node_role_)) continue;
            msg_queue_.push(msg);
        }

        if(not msg_queue_.available())
            return None;

        return Some(msg_queue_.pop());
    };


    TrackTarget track_target = {
        .roll = {.position = 0, .speed = 0},
        .pitch = {.position = 0, .speed = 0}
    };

    [[maybe_unused]] auto set_target = [&](const SetPositionAndSpeed & cmd){
        track_pos_ = real_t(cmd.position);
        track_spd_ = real_t(cmd.speed);
    };

    [[maybe_unused]] auto can_subscriber_service = [&]{
        static async::RepeatTimer timer{5ms};

        timer.invoke_if([&]{
            auto process_msg = [&](const hal::CanMsg & msg){
                const auto id = msg.stdid().unwrap();
                const auto [msg_role, msg_cmd] = dump_role_and_cmd(id);
                if(msg_role != self_node_role_) return;
                switch(msg_cmd){
                    case CommandKind::SetPositionAndSpeed:{
                        const auto cmd = serde::make_deserialize<serde::RawBytes,
                            SetPositionAndSpeed>(msg.iter_payload()).examine();
                        set_target(cmd);
                    }
                        break;

                    default:
                        PANIC("unknown command", std::bit_cast<uint8_t>(msg_cmd));
                        break;
                }
            };

            while(true){
                const auto may_msg = read_can_msg();
                if(may_msg.is_none()) break;
                process_msg(may_msg.unwrap());
            }
        });
    };


    [[maybe_unused]] auto can_publisher_service = [&]{
        static async::RepeatTimer timer{5ms};

        auto publish_roll_joint_target = [&]{
            const auto msg = MsgFactory{NodeRole::RollJoint}(track_target.roll);

            write_can_msg(msg);
        };

        auto publish_pitch_joint_target = [&]{
            const auto msg = MsgFactory{NodeRole::PitchJoint}(track_target.pitch);

            write_can_msg(msg);
        };

        timer.invoke_if([&]{

            switch(self_node_role_){
                case NodeRole::PitchJoint:
                    break;
                case NodeRole::RollJoint:{
                    publish_roll_joint_target();
                    publish_pitch_joint_target();
                    break;
                default:
                    __builtin_unreachable();
                    break;
                }
            }
        });
    };


    [[maybe_unused]] auto repl_service = [&]{
        static robots::ReplServer repl_server{&DBG_UART, &DBG_UART};

        static const auto list = rpc::make_list(
            "list",
            rpc::make_function("rst", [](){sys::reset();}),
            rpc::make_function("outen", [&](){repl_server.set_outen(EN);}),
            rpc::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),

            rpc::make_property_with_limit("kp", &pd_ctrl_law_.kp, 0, 10),
            rpc::make_property_with_limit("kd", &pd_ctrl_law_.kd, 0, 10),

            rpc::make_function("pp", [&](const real_t p1, const real_t p2){

                track_target.roll.position   = CLAMP2(p1, JOINT_POSITION_LIMIT);
                track_target.pitch.position  = CLAMP2(p2, JOINT_POSITION_LIMIT);
            })
            
        );

        repl_server.invoke(list);
    };

    [[maybe_unused]] auto demo_track_service = [&]{ 
        auto update_joint_target = [&]() -> void { 
            const auto clock_time = clock::time();
            const auto [s, c] = sincos(clock_time * 5);
            const auto p1 = c * 0.00_r;
            const auto p2 = s * 0.00_r;

            track_target.roll.position   = CLAMP2(p1, JOINT_POSITION_LIMIT);
            track_target.pitch.position  = CLAMP2(p2, JOINT_POSITION_LIMIT);
        };

        update_joint_target();
    };  

    [[maybe_unused]] auto gesture_calc_service = [&]{
        [[maybe_unused]] static constexpr auto DELTA_TIME_MS = 5ms;
        [[maybe_unused]] static constexpr auto DELTA_TIME = DELTA_TIME_MS.count() * 0.001_r;
        [[maybe_unused]] static constexpr size_t FREQ = 1000ms / DELTA_TIME_MS;

        static async::RepeatTimer timer{DELTA_TIME_MS};
        static auto gesture_estimator = GestureEstimator({.fs = FREQ});

        timer.invoke_if([&]{
            const auto acc = bmi.read_acc().examine();
            const auto gyr = bmi.read_gyr().examine();

            gesture_estimator.process(acc, gyr);

            std::tie(base_roll_theta_, base_roll_omega_) = gesture_estimator.theta_and_omega();
        });
    };


    while(true){
        repl_service();
        demo_track_service();
        gesture_calc_service();

        can_publisher_service();
        can_subscriber_service();

        blink_service();


        #if 0
        // const auto command = SetPositionAndSpeed{2, 18};
        // const auto iter = make_serialize_iter<RawBytes>(command);
        // const auto iter = make_serialize_iter();

        std::array<char, 16> arr{};

        const auto u_begin = clock::micros();
        const auto v = 20 * sin(7 * clock::time());
        // const auto v = 10.1_r;
        // const auto v = 10;

        for(size_t i = 0; i < 100; i++){
            strconv2::to_str(v, StringRef{arr.data(), arr.size()}).examine();
        }

        // static constexpr auto msg = MsgFactory{NodeRole::Master}(SetPositionAndSpeed{0, 1});
        // static constexpr auto des = serde::make_deserializer<serde::RawBytes, SetPositionAndSpeed>();
        // static constexpr auto may_cmd = des.deserialize(std::span(msg.payload().data(), msg.size()));
        // const auto rem_str = strconv2::to_str(v, StringRef{arr.data(), arr.size()}).examine();
        // auto iter = serde::make_serialize_iter<serde::RawBytes>(SetPositionAndSpeed{13 *256 + 12, 11});
        // auto iter = serde::make_serialize_iter<serde::RawBytes>(std::make_tuple<real_t, real_t>(13 *256 + 12, 11));
        // auto iter = serde::make_serialize_iter<serde::RawBytes>(std::make_tuple<int32_t, int32_t>(
        //     (15 << 24) + (14 << 16) + 13 *256 + 12, 11));
        // DEBUG_PRINTLN(command, iter, SetKpKdCommand{.kp = 1, .kd = 1});
        auto iter = serde::make_serialize_iter<serde::RawBytes>(
                robots::machine_cmds::Replace{
                    .x1 = 0_bf16,
                    .y1 = 1_bf16,
                    .x2 = 0_bf16,
                    .y2 = 0_bf16
                }
            );

        [[maybe_unused]] auto msg = hal::CanMsg::from_iter(hal::CanStdId(0),iter).unwrap();
        // auto res = strconv2::to_str<uint8_t>(100, StringRef{arr.data(), arr.size()});
        // auto res = strconv2::TostringResult(Ok(uint8_t(100)));
        // uint8_t res = 100;
        // DEBUG_PRINTLN(;
        // DEBUG_PRINTLN(StringView(arr.data(), arr.size()));
        #endif

        #if 0
        DEBUG_PRINTLN_IDLE(
            // (clock::micros() - u_begin).count(), 
            // StringView(arr.data()), 
            // rem_str.size(),
            // base_roll_theta_,
            // base_roll_omega_,
            // iter,
            msg.iter_payload(),
            serde::make_deserializer<serde::RawBytes, robots::machine_cmds::Replace>()
                .deserialize(msg.iter_payload()).examine()


            // count_iter(iter),
            // (MsgFactory{NodeRole::Master})(SetPositionAndSpeed{0, 1}).payload()
            // strconv2::iq_from_str<16>("+.").examine()
        );
        #endif

        #if 0
        // for(const auto item: range){
        //     DEBUG_PRINTLN_IDLE(item, '?');
        // }

        // [[maybe_unused]] const auto t = clock::time();
        // [[maybe_unused]] const auto uvw_curr = curr_sens.uvw();
        // [[maybe_unused]] const auto dq_curr = curr_sens.dq();
        // [[maybe_unused]] const auto ab_curr = curr_sens.ab();

        // DEBUG_PRINTLN_IDLE(odo.getPosition(), iq_t<16>(speed_measurer.result()), sin(t), t);
        // if(false)


        // // static constexpr auto a = sizeof(iter);
        // DEBUG_PRINTLN_IDLE(
        //     // pos_sensor_.position(),
        //     // pos_sensor_.cont_position(),
        //     // pos_sensor_.lap_position(),
        //     // pos_sensor_.speed(),
        //     // leso.get_disturbance(),
        //     base_roll_raw,
        //     base_roll_theta_,
        //     range,
        //     // base_roll_omega_,
        //     // len_acc,
        //     alpha,
        //     // iter,
        //     // meas_rad_
        //     exe_us_.count()
        // );

        #endif
    }
}



[[maybe_unused]]
static void static_test(){
    #define STATIC_TEST_SEL 5
    #if STATIC_TEST_SEL == 0
    static constexpr hal::CanMsg msg = (MsgFactory{NodeRole::Master})(SetPositionAndSpeed{0, 1});
    static_assert(msg.size() == 8);
    static constexpr auto des = serde::make_deserializer<serde::RawBytes, SetPositionAndSpeed>();
    static constexpr auto may_cmd = des.deserialize(std::span(msg.payload().data(), msg.size()));
    // static_assert(may_cmd.is_ok());
    static_assert(int(may_cmd.unwrap_err()) == int(serde::DeserializeError::BytesLengthLong));
    #elif STATIC_TEST_SEL ==1
    static constexpr auto msg = (MsgFactory{NodeRole::Master})(SpinCommand{1});
    static constexpr auto des = serde::make_deserializer<serde::RawBytes, SpinCommand>();
    static constexpr auto may_cmd = des.deserialize(std::span(msg.payload().data(), msg.size()));
    static constexpr auto cmd = may_cmd.unwrap();
    static constexpr auto rot = cmd.delta_rotation;
    static_assert(msg.size() == 4);
    static_assert(rot == 1.0_q16);
    // static_assert(int(may_cmd.unwrap_err()) == int(serde::DeserializeError::BytesLengthLong));
    #elif STATIC_TEST_SEL == 2
    // static constexpr hal::CanMsg msg = (MsgFactory{NodeRole::Master})(SetPositionXYZCommand{0, 1, 2});
    static constexpr hal::CanMsg msg = hal::CanMsg::from_iter(hal::CanStdId(0), MyIter<8>{}).unwrap();
    static_assert(msg.size() == 12);
    static constexpr auto des = serde::make_deserializer<serde::RawBytes, SetPositionXYZCommand>();
    static constexpr auto may_cmd = des.deserialize(std::span(msg.payload().data(), msg.size()));
    // static_assert(may_cmd.is_ok());
    static_assert(int(may_cmd.unwrap_err()) == int(serde::DeserializeError::BytesLengthLong));
    #endif
}