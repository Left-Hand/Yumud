#include "src/testbench/tb.h"
#include <atomic>


#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/polymorphism/traits.hpp"
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
#include "drivers/GateDriver/MP6540/mp6540.hpp"

#include "types/vectors/quat.hpp"


#include "robots/gesture/comp_est.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"
#include "robots/protocols/cannet/can_chain.hpp"

#include "robots/commands/joint_commands.hpp"
#include "robots/commands/machine_commands.hpp"
#include "robots/commands/nmt_commands.hpp"
#include "robots/nodes/msg_factory.hpp"

#include "types/regions/perspective_rect.hpp"

#include "dsp/motor_ctrl/position_filter.hpp"
#include "dsp/motor_ctrl/ctrl_law.hpp"
#include "dsp/motor_ctrl/elecrad_compsator.hpp"
#include "dsp/controller/adrc/leso.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "digipw/prelude/abdq.hpp"


#include "primitive/analog_channel.hpp"

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
static constexpr auto DELTA_TIME = DELTA_TIME_MS.count() * 0.001_iq20;

static constexpr uint32_t CHOPPER_FREQ = 25000;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;
static constexpr size_t CANMSG_QUEUE_SIZE = 8;


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

enum class BlinkPattern:uint8_t{
    RGB,
    RED,
    GREEN,
    BLUE
};


void bldc_main(){
    const auto UART_BAUD = 576000;

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

    auto ma730_cs_gpio_ = hal::PA<15>();

    auto & pwm_u = timer1.oc<1>();
    auto & pwm_v = timer1.oc<2>();
    auto & pwm_w = timer1.oc<3>(); 

    led_red_gpio_.outpp(); 
    led_blue_gpio_.outpp(); 
    led_green_gpio_.outpp();


    clock::delay(2ms);

    can.init({
        .remap = CAN1_REMAP,
        .mode = hal::CanMode::Normal,
        .timming_coeffs = hal::CanBaudrate(hal::CanBaudrate::_1M).to_coeffs()
    });

    can.filters<0>().apply(hal::CanFilterConfig::from_accept_all());

    spi.init({
        .baudrate = 18_MHz
    });


    timer1.init({
        .count_freq = hal::NearestFreq(CHOPPER_FREQ * 2), 
        .count_mode = hal::TimerCountMode::CenterAlignedUpTrig
    }, EN);

    timer1.oc<4>().init({.plant_en = DISEN});
    timer1.oc<4>().cvr() = timer1.arr() - 1;

    pwm_u.init({});
    pwm_v.init({});
    pwm_w.init({});

    led_red_gpio_.outpp(); 
    led_blue_gpio_.outpp(); 
    led_green_gpio_.outpp();
    mp6540_nfault_gpio_.inana();

    can.init({
        .remap = CAN1_REMAP,
        .mode = hal::CanMode::Normal,
        .timming_coeffs = hal::CanBaudrate(hal::CanBaudrate::_1M).to_coeffs()
    });



    MA730 ma730_{
        &spi,
        spi.allocate_cs_gpio(&ma730_cs_gpio_)
            .unwrap()
    };



    ma730_.init({
        .direction = CW
    }).examine();

    mp6540_en_gpio_.outpp(HIGH);
    mp6540_nslp_gpio_.outpp(HIGH);
    static constexpr auto mp6540_adc_scaler = MP6540::make_adc_scaler(10'000);

    [[maybe_unused]] auto u_sense = hal::ScaledAnalogInput(adc.inj<1>(), mp6540_adc_scaler);
    [[maybe_unused]] auto v_sense = hal::ScaledAnalogInput(adc.inj<2>(), mp6540_adc_scaler);
    [[maybe_unused]] auto w_sense = hal::ScaledAnalogInput(adc.inj<3>(), mp6540_adc_scaler);
    
    auto uvw_pwmgen = UvwPwmgen(&pwm_u, &pwm_v, &pwm_w);
    

    init_adc(adc);

    hal::PA<7>().inana();

    AlphaBetaCoord<iq16> ab_volt_;
    
    dsp::PositionFilter pos_filter_{
        typename dsp::PositionFilter::Config{
            .fs = FOC_FREQ,
            .r = 105
        }
    };

    pos_filter_.set_base_lap_angle(
        Angle<iq16>::ZERO
    );

    ElecAngleCompensator elec_angle_comp_{
        .base = 0,
        .pole_pairs = 7
    };

    dsp::Leso leso_{
        dsp::Leso::Config{
            .b0 = 0.5_r,
            .w = 0.4_r,
            .fs = FOC_FREQ
        }
    };

    auto pd_ctrl_law_ = PdCtrlLaw{.kp = 128.581_r, .kd = 18.7_r};

    iq20 q_volt_ = 0;
    Angle<iq20> meas_elec_angle_ = Angle<iq20>::ZERO;

    [[maybe_unused]] iq20 axis_target_position_ = 0;
    [[maybe_unused]] iq20 axis_target_speed_ = 0;
    
    Microseconds exe_us_ = 0us;

    RunStatus run_status_;
    run_status_.state = RunState::Tracking;

    bool report_en_ = true;

    auto update_sensors = [&]{ 
        ma730_.update().examine();

        const auto meas_lap_angle = ma730_.read_lap_angle().examine(); 
        pos_filter_.update(Angle<iq16>::from_turns(meas_lap_angle.to_turns()));
    };

    auto sensored_foc_cb = [&]{
        update_sensors();

        const auto meas_lap_angle = ma730_.read_lap_angle().examine(); 
        const auto meas_elec_angle = elec_angle_comp_(meas_lap_angle);

        [[maybe_unused]] const auto meas_position = pos_filter_.accumulated_angle().to_turns();
        [[maybe_unused]] const auto meas_speed = pos_filter_.speed();

        [[maybe_unused]] static constexpr iq20 omega = 2;
        [[maybe_unused]] static constexpr iq20 amp = 0.300_r;
        [[maybe_unused]] const auto ctime = clock::time();

        const auto [axis_target_position, axis_target_speed] = ({
            std::make_tuple(
                amp * sin(omega * ctime), amp * omega * cos(omega * ctime)
                // axis_target_position_, axis_target_speed_
            );
        });


        [[maybe_unused]] const auto targ_position = axis_target_position;
        [[maybe_unused]] const auto targ_speed = axis_target_speed;


        #if 1
        const auto q_volt = 3.3_r;

        [[maybe_unused]] const auto alphabeta_volt = DqCoord<iq16>{
            .d = 0, 
            .q = q_volt
        }.to_alphabeta(Rotation2<iq16>::from_angle(Angle<iq16>::from_turns(ctime)));
        #else
        const auto q_volt = CLAMP2(
            pd_ctrl_law_(targ_position - meas_position, targ_speed - meas_speed)
        , SVPWM_MAX_VOLT);

        [[maybe_unused]] const auto alphabeta_volt = DqCoordVoltage{
            0, 
            CLAMP2(q_volt - leso_.get_disturbance(), SVPWM_MAX_VOLT)
        }.to_alphabeta(meas_elec_angle);
        #endif
        static constexpr auto INV_BUS_VOLT = iq16(1.0/12);

        uvw_pwmgen.set_dutycycle(
            SVM(alphabeta_volt * INV_BUS_VOLT)
        );

        leso_.update(meas_speed, q_volt);

        q_volt_ = q_volt;
        meas_elec_angle_ = meas_elec_angle;
    };


    adc.register_nvic({0,0}, EN);
    adc.enable_interrupt<hal::AdcIT::JEOC>(EN);
    adc.set_event_callback(
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

    [[maybe_unused]] auto repl_service = [&]{
        static robots::ReplServer repl_server{&DBG_UART, &DBG_UART};

        static const auto list = rpc::make_list(
            "list",
            rpc::make_function("rst", [](){sys::reset();}),
            rpc::make_function("outen", [&](){repl_server.set_outen(EN);}),
            rpc::make_function("outdis", [&](){repl_server.set_outen(DISEN);}),



            rpc::make_property_with_limit("kp", &pd_ctrl_law_.kp, 0, 240),

            rpc::make_property_with_limit("kd", &pd_ctrl_law_.kd, 0, 30),

            rpc::make_function("setp", [&](real_t p){axis_target_position_ = p;}),

            rpc::make_function("mp", [&](){
                DEBUG_PRINTLN(axis_target_position_, pos_filter_.accumulated_angle().to_turns());
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

    while(true){
        static auto timer = async::RepeatTimer::from_duration(1000ms / MACHINE_CTRL_FREQ);
        timer.invoke_if([&]{
            blink_service();
        });

        repl_service();

        if(report_en_){
            static auto report_timer = async::RepeatTimer::from_duration(DELTA_TIME_MS);
            
            report_timer.invoke_if([&]{

                DEBUG_PRINTLN_IDLE(
                    // pos_filter_.cont_position(), 
                    // pos_filter_.speed(),
                    // meas_elec_angle_,
                    // q_volt_,
                    pwm_u.get_dutycycle(),
                    pwm_v.get_dutycycle(),
                    pwm_w.get_dutycycle()
                    // pos_filter_.position(),
                    // pos_filter_.speed(),
                    // axis_target_position_,
                    // axis_target_speed_,
                    // may_err_position_
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