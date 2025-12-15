#include "src/testbench/tb.h"

#include <atomic>

#include "core/debug/debug.hpp"
#include "core/async/timer.hpp"
#include "core/utils/sumtype.hpp"
#include "core/string/utils/strconv2.hpp"
#include "core/utils/combo_counter.hpp"
#include "core/utils/default.hpp"
#include "core/async/delayed_semphr.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/analog/opa/opa.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/IMU/Axis6/BMI160/BMI160.hpp"
#include "drivers/GateDriver/MP6540/mp6540.hpp"

#include "algebra/vectors/quat.hpp"

#include "robots/gesture/comp_est.hpp"
#include "middlewares/rpc/rpc.hpp"
#include "middlewares/rpc/repl_server.hpp"

#include "dsp/motor_ctrl/position_filter.hpp"
#include "dsp/motor_ctrl/sensored/calibrate_table.hpp"
#include "dsp/motor_ctrl/ctrl_law.hpp"
#include "dsp/motor_ctrl/elecrad_compsator.hpp"
#include "dsp/controller/adrc/linear/leso2o.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "digipw/prelude/abdq.hpp"

#include "nuedc_2025e_common.hpp"



using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::digipw;
using namespace ymd::dsp;
using namespace ymd::robots;
using namespace ymd::nuedc_2025e;

using robots::NodeRole;
using robots::MsgFactory;


static constexpr auto SVPWM_MAX_VOLT = 3.87_r;

static constexpr uint32_t CHOPPER_FREQ = 25000;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;

enum class BlinkPattern:uint8_t{
    RGB,
    RED,
    GREEN,
    BLUE
};

static constexpr size_t UART_BAUD = 576000;


[[maybe_unused]] static void init_adc(hal::AdcPrimary & adc){

    using hal::AdcChannelSelection;
    using hal::AdcSampleCycles;

    adc.init(
        {
            {AdcChannelSelection::VREF, AdcSampleCycles::T28_5}
        },{
            {AdcChannelSelection::CH5, AdcSampleCycles::T13_5},
            {AdcChannelSelection::CH4, AdcSampleCycles::T13_5},
            {AdcChannelSelection::CH1, AdcSampleCycles::T13_5},
            {AdcChannelSelection::VREF, AdcSampleCycles::T7_5},
        }, {}
    );

    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1CC4);
    adc.enable_auto_inject(DISEN);
}



void nuedc_2025e_joint_main(){
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



    auto & DBG_UART = hal::uart2;

    DBG_UART.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
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

    auto led_blue_pin_ = hal::PC<13>();
    auto led_red_pin_ = hal::PC<14>();
    auto led_green_pin_ = hal::PC<15>();

    auto mp6540_en_pin_ = hal::PB<15>();
    auto mp6540_nslp_pin_ = hal::PB<14>();
    auto mp6540_nfault_pin_ = hal::PA<7>();
    auto ma730_cs_pin_ = hal::PA<0>();

    
    auto & pwm_u = timer1.oc<1>();
    auto & pwm_v = timer1.oc<2>();
    auto & pwm_w = timer1.oc<3>(); 

    timer1.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15,
        .count_freq = hal::NearestFreq(CHOPPER_FREQ * 2), 
        .count_mode = hal::TimerCountMode::CenterAlignedUpTrig
    })
        .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();
    timer1.start();

    timer1.oc<4>().init([&]{
        auto config = timer1.oc<4>().default_config();
        return config;
    }());

    timer1.oc<4>().cvr() = timer1.arr() - 1;

    pwm_u.init(Default);
    pwm_v.init(Default);
    pwm_w.init(Default);

    led_red_pin_.outpp(); 
    led_blue_pin_.outpp(); 
    led_green_pin_.outpp();
    mp6540_nfault_pin_.inana();

    const auto self_node_role_ = get_node_role(chip_id_crc_)
        .expect(chip_id_crc_);

    can.init({
        .remap = hal::CAN1_REMAP_PA12_PA11,
        .wiring_mode = hal::CanWiringMode::Normal,
        .bit_timming = hal::CanBaudrate(hal::CanBaudrate::_1M), 
    });

    can.filters<0>() 
        .apply(hal::CanFilterConfig::from_pair(
            hal::CanStdIdMaskPair::from_parts(
                comb_role_and_cmd(self_node_role_, uint8_t(0x00)), 
                hal::CanStdId::from_bits(0b1111'000'0000), hal::CanRtrSpecfier::Discard
            ))
        )
    ;

    spi.init({
        .remap = hal::SPI1_REMAP_PA5_PA6_PA7_PA4,
        .baudrate = hal::NearestFreq(18_MHz)
    });


    clock::delay(2ms);

    MA730 ma730_{
        &spi,
        spi.allocate_cs_pin(&ma730_cs_pin_)
            .unwrap()
    };

    ma730_.init({
        .direction = CW
    }).examine();

    bmi160::BMI160 bmi160_{
        &spi,
        spi.allocate_cs_pin(&ma730_cs_pin_)
            .unwrap()
    };

    if(self_node_role_ == NodeRole::YawJoint){
        bmi160_.init({
            .acc_odr = bmi160::AccOdr::_200Hz,
            .acc_fs = bmi160::AccFs::_16G,
            .gyr_odr = bmi160::GyrOdr::_200Hz,
            .gyr_fs = bmi160::GyrFs::_500deg

            // .acc_odr = BMI160::AccOdr::_400Hz,
            // .acc_fs = BMI160::AccFs::_16G,
            // .gyr_odr = BMI160::GyrOdr::_400Hz,
            // .gyr_fs = BMI160::GyrFs::_500deg
        }).examine();
    }

    static constexpr auto mp6540_adc_scaler = MP6540::make_adc_scaler(10'000);

    [[maybe_unused]] auto u_sense = hal::ScaledAnalogInput(adc.inj<1>(), mp6540_adc_scaler);
    [[maybe_unused]] auto v_sense = hal::ScaledAnalogInput(adc.inj<2>(), mp6540_adc_scaler);
    [[maybe_unused]] auto w_sense = hal::ScaledAnalogInput(adc.inj<3>(), mp6540_adc_scaler);
    
    auto uvw_pwmgen = UvwPwmgen(&pwm_u, &pwm_v, &pwm_w);

    init_adc(adc);
    hal::PA<7>().inana();

    mp6540_en_pin_.outpp(HIGH);
    mp6540_nslp_pin_.outpp(HIGH);

    iq20 q_volt_ = 0;
    Angular<uq32> meas_elec_angle_ = Angular<uq32>::ZERO;

    iq20 axis_target_position_ = 0;
    iq20 axis_target_speed_ = 0;

    Microseconds exe_us_ = 0us;

    RunStatus run_status_;
    run_status_.state = RunState::Idle;

    RingBuf<hal::BxCanFrame, CANMSG_QUEUE_SIZE> msg_queue_;

    AlphaBetaCoord<iq16> ab_volt_;
    
    dsp::PositionFilter pos_filter_{
        typename dsp::PositionFilter::Config{
            .fs = FOC_FREQ,
            .r = 105
        }
    };

    pos_filter_.set_base_lap_angle(
        
        Angular<iq16>::from_turns([&] -> iq16{
            switch(self_node_role_){
                case NodeRole::YawJoint:
                    return {0.389_r + 0.5_r};
                case NodeRole::PitchJoint:
                    return {0.737_r};
                    // return {0.223_r};
                default:
                    PANIC();
            }
        }())
    );

    ElecAngleCompensator elecangle_comp_{
        .base = [&]{
            switch(self_node_role_){
                case NodeRole::YawJoint:
                    return uq32(1-0.211);
                case NodeRole::PitchJoint:
                    return uq32(1-0.244);
                default:
                    PANIC();
            }
        }(),
        .pole_pairs = [&] -> uint32_t{
            return 7;
        }(),
    };

    using Leso = adrc::LinearExtendedStateObserver<iq20, 2>;
    Leso leso_{
        [&]{
            switch(self_node_role_){
                case NodeRole::YawJoint:
                    return Leso::Config{
                        .fs = FOC_FREQ,
                        .b0 = 0.5_r,
                        .w = 0.4_r
                    };
                case NodeRole::PitchJoint:
                    return Leso::Config{
                        .fs = FOC_FREQ,
                        .b0 = 1.4_r,
                        .w = 0.2_r
                    };

                default:
                    PANIC();
            }
        }()
    };

    PdCtrlLaw pd_ctrl_law_ = [&]{ 
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

    auto update_sensors = [&]{ 
        ma730_.update().examine();

        if(self_node_role_ == NodeRole::YawJoint){
            bmi160_.update().examine();
        }

        const auto meas_lap_angle = ma730_.read_lap_angle().examine(); 
        pos_filter_.update(Angular<iq16>::from_turns(meas_lap_angle.to_turns()));
    };


    auto sensored_foc_cb = [&]{
        update_sensors();

        if(run_status_.state == RunState::Idle){
            uvw_pwmgen.set_dutycycle(UvwCoord<iq16>::ZERO);
            // leso_.reset();
            return;
        }

        const auto meas_lap_angle = ma730_.read_lap_angle().examine(); 
        const auto meas_elec_angle = elecangle_comp_(meas_lap_angle);

        const auto meas_position = pos_filter_.accumulated_angle().to_turns();
        const auto meas_speed = pos_filter_.speed();

        const auto [axis_target_position, axis_target_speed] = ({
            std::make_tuple(
                axis_target_position_, axis_target_speed_
            );
        });


        const auto targ_position = axis_target_position;
        const auto targ_speed = axis_target_speed;


        const auto q_volt = CLAMP2(
            pd_ctrl_law_(targ_position - meas_position, targ_speed - meas_speed)
        , SVPWM_MAX_VOLT);

        [[maybe_unused]] const auto alphabeta_volt = DqCoord<iq16>{
            .d = 0, 
            .q = CLAMP2(q_volt, SVPWM_MAX_VOLT)
            // CLAMP2(q_volt, SVPWM_MAX_VOLT)
        }.to_alphabeta(Rotation2<iq20>::from_angle(meas_elec_angle));


        static constexpr auto INV_BUS_VOLT = iq16(1.0/12);

        uvw_pwmgen.set_dutycycle(
            SVM(alphabeta_volt * INV_BUS_VOLT)
        );

        // leso_.update(meas_speed, q_volt);

        q_volt_ = q_volt;
        meas_elec_angle_ = meas_elec_angle;
    };

    adc.register_nvic({0,0}, EN);
    adc.enable_interrupt<hal::AdcIT::JEOC>(EN);
    adc.set_event_handler(
        [&](const hal::AdcEvent ev){
            switch(ev){
            case hal::AdcEvent::EndOfInjectedConversion:{
                const auto m = clock::micros();
                sensored_foc_cb();
                exe_us_ = clock::micros() - m;
                break;
            }
            default: break;
            }
        }
    );
    

    auto joint_start_seeking = [&]{
        run_status_.state = RunState::Seeking;
    };

    auto joint_start_tracking = [&]{
        run_status_.state = RunState::Tracking;
    };

    auto joint_to_idle = [&]{
        run_status_.state = RunState::Idle;
    };


    auto read_can_frame = [&] -> Option<hal::BxCanFrame>{
        while(can.available()){
            auto frame = can.read();
            if(frame.is_extended()) continue;
            (void)msg_queue_.try_push(frame);
        }

        if(not msg_queue_.length())
            return None;

        hal::BxCanFrame frame = hal::BxCanFrame::from_uninitialized();
        if(msg_queue_.try_pop(frame) == 0)
            return None;
        return Some(frame);
    };

    [[maybe_unused]] auto delta_target_position_by_command = 
        [&](const commands::DeltaPosition & cmd){
        switch(self_node_role_){
            case NodeRole::YawJoint:{
                static constexpr auto THIS_BMI160_YAW_GYR_BIAS = 0.0020_iq24;

                // imu补偿
                const auto yaw_gyr = -(
                    bmi160_.read_gyr().examine().z + 
                    THIS_BMI160_YAW_GYR_BIAS);

                const auto yaw_speed = yaw_gyr * iq20(1 / TAU);

                const auto cmd_delta_position = iq20(cmd.delta_position + yaw_speed / MACHINE_CTRL_FREQ);
                axis_target_speed_ = iq20(cmd_delta_position * MACHINE_CTRL_FREQ);
                axis_target_position_ = (pos_filter_.accumulated_angle().to_turns() + cmd_delta_position);
                break;
            }
            case NodeRole::PitchJoint:{
                const auto cmd_delta_position = iq20(cmd.delta_position);
                axis_target_speed_ = iq20(cmd_delta_position * MACHINE_CTRL_FREQ);
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
                axis_target_position_ = iq20(cmd.position);
                break;
            }
            case NodeRole::PitchJoint:{
                axis_target_speed_ = 0;
                axis_target_position_ = CLAMP(
                    iq20(cmd.position),
                    PITCH_MIN_POSITION, 
                    PITCH_MAX_POSITION
                );
                break;
            }
            default:
                __builtin_unreachable();
        }
    };

    auto can_subscriber_service = [&]{
        auto dispatch_msg = [&](const CommandKind kind,const std::span<const uint8_t> payload){

            switch(kind){
            case CommandKind::ResetNode:
                sys::reset();
                break;

            case CommandKind::DeltaPosition:{
                const auto may_cmd = serde::deserialize<serde::RawLeBytes,
                    commands::DeltaPosition>(payload);
                if(may_cmd.is_ok()) delta_target_position_by_command(may_cmd.unwrap());
            }
                break;

            case CommandKind::SetPosition:{
                const auto may_cmd = serde::deserialize<serde::RawLeBytes,
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

            default:
                PANIC("invalid command", std::bit_cast<uint8_t>(kind));
                break;
            }
        };

        auto handle_msg = [&](const hal::BxCanFrame & frame){
            const auto stdid = frame.identifier().try_to_stdid().examine();
            const auto [msg_role, msg_cmd] = dump_role_and_cmd<CommandKind>(stdid);
            if(msg_role != self_node_role_) return;
            dispatch_msg(msg_cmd, frame.payload_bytes());
        };

        while(true){
            const auto may_msg = read_can_frame();
            if(may_msg.is_none()) break;
            const auto & frame = may_msg.unwrap();
            handle_msg(frame);
        }
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
                led_red_pin_ = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                led_blue_pin_ = LOW;
                led_green_pin_ = LOW;
                break;
            case BlinkPattern::BLUE:
                led_red_pin_ = LOW;
                led_blue_pin_ = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                led_green_pin_ = LOW;
                break;
            case BlinkPattern::GREEN:
                led_red_pin_ = LOW;
                led_blue_pin_ = LOW;
                led_green_pin_ = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                break;
            case BlinkPattern::RGB:
                led_red_pin_ = BoolLevel::from((uint32_t(clock::millis().count()) % 200) > 100);
                led_blue_pin_ = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
                led_green_pin_ = BoolLevel::from((uint32_t(clock::millis().count()) % 800) > 400);
                break;
        }
    };


    while(true){
        blink_service();
        can_subscriber_service();

    
        static auto report_timer = async::RepeatTimer::from_duration(DELTA_TIME_MS);
        
        report_timer.invoke_if([&]{

            DEBUG_PRINTLN_IDLE(
                // pos_filter_.cont_position(), 
                // pos_filter_.speed(),
                // meas_elec_angle_,
                // q_volt_,
                // pos_filter_.position(),
                // pos_filter_.speed(),
                axis_target_position_
                // axis_target_speed_,
                // math::sinpu(now_secs) / MACHINE_CTRL_FREQ * 0.2_r
                // yaw_angle_,
                // pos_filter_.lap_position(),
                // run_status_.state
                // may_center
            );
        });
    }
}