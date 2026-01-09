#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/utils/default.hpp"

#include "hal/timer/hw_singleton.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/analog/adc/hw_singleton.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/analog/opa/opa.hpp"
#include "hal/dma/dma.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "digipw/prelude/abdq.hpp"
#include "digipw/ctrl/pi_controller.hpp"

#include "drivers/GateDriver/DRV832X/DRV8323h.hpp"
#include "drivers/Encoder/MagEnc/MT6825/mt6825.hpp"
#include "drivers/Encoder/MagEnc/VCE2755/vce2755.hpp"
#include "drivers/GateDriver/uvw_pwmgen.hpp"


#include "dsp/motor_ctrl/sensorless/slide_mode_observer.hpp"
#include "dsp/motor_ctrl/sensorless/luenberger_observer.hpp"
#include "dsp/motor_ctrl/sensorless/nonlinear_flux_observer.hpp"
#include "dsp/controller/adrc/nonlinear/nltd2o.hpp"
#include "dsp/controller/adrc/linear/ltd2o.hpp"
#include "dsp/filter/firstorder/lpf.hpp"
#include "dsp/filter/butterworth/band.hpp"

#include "middlewares/rpc/rpc.hpp"
#include "middlewares/rpc/repl_server.hpp"
#include "core/async/timer.hpp"

#include "linear_regression.hpp"


//电机参数：
// https://item.taobao.com/item.htm?id=643573104607
using namespace ymd;

using namespace ymd::drivers;
using namespace ymd::digipw;
using namespace ymd::dsp;
using namespace ymd::dsp::adrc;


namespace ymd::dsp::adrc{



struct [[nodiscard]] MotorLeso{
public:

    struct [[nodiscard]] Coeffs{
        uq8 b0;
        uq32 dt;
        uq32 g1t;
        uq16 g2t;

        friend OutputStream & operator <<(OutputStream & os, const Coeffs & coeffs){
            return os << os.field("b0")(coeffs.b0) << os.splitter()
                << os.field("dt")(coeffs.dt) << os.splitter()
                << os.field("g1t")(coeffs.g1t) << os.splitter()
                << os.field("g2t")(coeffs.g2t);
        }
    };

    struct [[nodiscard]] Config{
        uint32_t fs;
        uint32_t fc;
        uq8 b0;

        constexpr Result<Coeffs, const char *> try_into_coeffs() const{
            auto & self = *this;
            const auto dt = uq32::from_rcp(self.fs);
            if(self.fs >= 65536) 
                return Err("fs too large");
            if(self.fc * 2 >= fs ) 
                return Err("fc too large");

            const uq32 g1t = uq32::from_bits(static_cast<uint32_t>(
                2u * uint64_t(fc) * uint64_t(uint64_t(1) << 32) / fs));
            const uq16 g2t = uq16::from_bits(static_cast<uint32_t>(
                uint64_t(fc)  * uint64_t(fc) * uint64_t(uint64_t(1) << 16) / fs));
            return Ok(Coeffs{
                .b0 = self.b0,
                .dt = dt,
                .g1t = g1t,
                .g2t = g2t
            });
        }
    };

    using State = SecondOrderState<iq16>;

    constexpr explicit MotorLeso(const Coeffs & coeffs):
        coeffs_(coeffs){;}

    constexpr State iterate(const State & state, const iq16 y, const iq16 u) const {
        // dx1=x2+b0*u+g1*(y-x1);
        // dx2=g2*(y-x1);

        #if 0
        uq16 g1t = coeffs_.dt * coeffs_.g1;
        uq16 g2t = coeffs_.dt * coeffs_.g2;
        const auto e = (y - state.x1);
        const auto delta_x1 =  ((state.x2 + u * coeffs_.b0 ) * coeffs_.dt) + (e * g1t);
        const auto delta_x2 = (e * g2t);
        return State{
            state.x1 + delta_x1, 
            state.x2 + delta_x2
        };
        #else

        #if 1
        const auto e = (y - math::fixed_downcast<16>(state.x1));
        const auto delta_x1 = extended_mul((state.x2 + (u * coeffs_.b0)), coeffs_.dt)
            + extended_mul(e, coeffs_.g1t);
        const auto delta_x2 = (e * coeffs_.g2t);
        return State{
            state.x1 + delta_x1,
            state.x2 + delta_x2
        };
        #else
        const auto e = (y - state.x1);
        const auto delta_x1 = (state.x2 + u * coeffs_.b0 + e * coeffs_.g1) * coeffs_.dt;
        const auto delta_x2 = (e * coeffs_.g2) * coeffs_.dt;
        return State{
            state.x1 + delta_x1,
            state.x2 + delta_x2
        };
        #endif

        #endif
    }

private:
    using Self = MotorLeso;
    Coeffs coeffs_;
};

}

#define DBG_UART hal::usart2
using Leso = dsp::adrc::MotorLeso;
// static constexpr uint32_t DEBUG_UART_BAUD = 576000;
// static constexpr uint32_t CHOPPER_FREQ = 24_KHz;
static constexpr uint32_t CHOPPER_FREQ = 32_KHz;
static constexpr uint32_t FOC_FREQ = CHOPPER_FREQ;

static constexpr auto BUS_VOLT = iq16(12.0);
static constexpr auto INV_BUS_VOLT = 1 / BUS_VOLT;
static constexpr size_t HFI_FREQ = 1000;

struct MotorProfile_Gim6010{
    static constexpr size_t POLE_PAIRS = 10u;
    // static constexpr auto PHASE_INDUCTANCE = 0.0085_iq20;
    // static constexpr auto PHASE_INDUCTANCE = 0.00245_iq20;
    // static constexpr auto PHASE_INDUCTANCE = 0.0025_iq20;

    //100uh
    static constexpr auto PHASE_INDUCTANCE = iq20(22.3 * 1E-6);

    //1ohm
    // static constexpr auto PHASE_RESISTANCE = 1.123_iq20;
    static constexpr auto PHASE_RESISTANCE = 0.123_iq20;
};

struct MotorProfile_Ysc{
    static constexpr size_t POLE_PAIRS = 7u;
    static constexpr auto PHASE_INDUCTANCE = iq20(180 * 1E-6);
    // static constexpr auto PHASE_INDUCTANCE = 0.00325_iq20;
    static constexpr auto PHASE_RESISTANCE = 0.303_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq16>::from_turns(0.145_uq16);

    // static constexpr uint32_t CURRENT_CUTOFF_FREQ = 2400;
    static constexpr uint32_t CURRENT_CUTOFF_FREQ = 400;
    static constexpr auto MODU_VOLT_LIMIT = iq16(5.5);
    static constexpr auto leso_b0 = 30;

    static constexpr iq16 KP = 1.73_iq16;
    // const iq16 kd = 0.16_iq16;
    static constexpr iq16 KD = 0.075_iq16;
    static constexpr auto leso_coeffs = Leso::Config{
        .fs = FOC_FREQ,
        // .fc = 2000,
        .fc = 50,
        // .b0 = 1000
        .b0 = leso_b0
    }.try_into_coeffs().unwrap();

    using MagEncoder = MT6825;
};

struct MotorProfile_Gim4010{
    static constexpr size_t POLE_PAIRS = 14u;
    static constexpr auto PHASE_INDUCTANCE = iq20(300 * 1E-6);
    // static constexpr auto PHASE_INDUCTANCE = 0.00325_iq20;
    static constexpr auto PHASE_RESISTANCE = 1.03_iq20;
    static constexpr auto SENSORED_ELEC_ANGLE_BASE = Angular<uq16>::from_turns(0.265_uq16);
    static constexpr auto MODU_VOLT_LIMIT = iq16(7.5);
    static constexpr auto CURRENT_CUTOFF_FREQ = 1600;
    static constexpr auto leso_coeffs = Leso::Config{
        .fs = FOC_FREQ,
        // .fc = 2000,
        .fc = 50,
        // .b0 = 1000
        .b0 = 30
    }.try_into_coeffs().unwrap();
    static constexpr iq16 KP = 2.23_iq16;
    // const iq16 kd = 0.16_iq16;
    static constexpr iq16 KD = 0.045_iq16;
    using MagEncoder = VCE2755;
};

// using MotorProfile = MotorProfile_Ysc;
using MotorProfile = MotorProfile_Gim4010;


struct LrSeriesCurrentRegulatorConfig{
    uint32_t fs;                 // 采样频率 (Hz)
    uint32_t fc;                 // 截止频率/带宽 (Hz)
    iq16 phase_inductance;        // 相电感 (H)
    iq16 phase_resistance;        // 相电阻 (Ω)
    iq16 voltage_limit;                // 最大电压 (V)

    [[nodiscard]] constexpr Result<digipw::PiController::Cofficients, StringView> try_to_coeff() const {
        //U(s) = I(s) * R + s * I(s) * L
        //I(s) / U(s) = 1 / (R + sL)
        //G_open(s) = (Ki / s + Kp) / s(R / s + L)

        // Ki = 2pi * fc * R
        // Kp = 2pi * fc * L

        const auto & self = *this;
        digipw::PiController::Cofficients coeff;

        const auto norm_omega = iq16(iq16(TAU) * fc / self.fs);
        coeff.max_out = self.voltage_limit;

        coeff.kp = iq20(self.phase_inductance * self.fc) * iq16(TAU);
        coeff.ki_discrete = self.phase_resistance * norm_omega;

        // coeff.ki_discrete = 0;

        coeff.err_sum_max = self.voltage_limit / iq16(coeff.ki_discrete);
        return Ok(coeff);
    }
};




#if 0
[[maybe_unused]] auto speed_compansate_dq_volt = [&]{
    auto dq_volt = dq_volt_;
    dq_volt.d = 0.0_iq20;
    // dq_volt.d = 0.0005_iq20 * linear_speed;
    // dq_volt.d = 0.0005_iq20 * linear_speed;
    dq_volt.q = 0.0_iq20;
    return dq_volt;
};

#endif


static constexpr auto current_regulator_cfg = LrSeriesCurrentRegulatorConfig{
    .fs = FOC_FREQ,
    .fc = MotorProfile::CURRENT_CUTOFF_FREQ,
    .phase_inductance = MotorProfile::PHASE_INDUCTANCE,
    .phase_resistance = MotorProfile::PHASE_RESISTANCE,
    .voltage_limit = MotorProfile::MODU_VOLT_LIMIT,
};



static void init_adc(){

    hal::adc1.init({
            {hal::AdcChannelSelection::VREF, hal::AdcSampleCycles::T28_5}
        },{

            {hal::AdcChannelSelection::CH1, hal::AdcSampleCycles::T13_5},
            {hal::AdcChannelSelection::CH4, hal::AdcSampleCycles::T13_5},
            {hal::AdcChannelSelection::CH5, hal::AdcSampleCycles::T13_5},

        },
        {}
    );

    hal::adc1.set_injected_trigger(hal::AdcInjectedTrigger::T1CC4);
    hal::adc1.enable_auto_inject(DISEN);
}

using namespace ymd::math;

template<size_t Q>
static constexpr fixed_t<Q, int32_t> lpf_exprimetal(fixed_t<Q, int32_t> x_state, const fixed_t<Q, int32_t> x_new){
    constexpr uq32 alpha = uq32::from_bits(static_cast<uint32_t>((0.999) * (uint64_t(1u) << 32)));
    constexpr uq32 beta = uq32::from_bits(~alpha.to_bits());
    return fixed_t<Q, int32_t>::from_bits(
        static_cast<int32_t>(
            ((static_cast<int64_t>(x_state.to_bits()) * alpha.to_bits()) 
            + (static_cast<int64_t>(x_new.to_bits()) * beta.to_bits())) >> 32
        )
    );
}

void myesc_main(){
    DBG_UART.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(DEBUG_UART_BAUD),
        .tx_strategy = CommStrategy::Blocking,
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);
    // DEBUGGER.force_sync(EN);


    clock::delay(2ms);
    auto & timer = hal::timer1;

    // #region 初始化定时器

    static constexpr auto MOS_1C840L_500MA_BEST_DEADZONE = 90ns;
    // static constexpr auto MOS_1C840L_100MA_BEST_DEADZONE = 350ns;
    timer.bdtr().init({MOS_1C840L_500MA_BEST_DEADZONE});
    // timer.init_bdtr(MOS_1C840L_100MA_BEST_DEADZONE);

    timer.init({
        .remap = hal::TIM1_REMAP_A8_A9_A10_A11__A7_B0_B1,
        .count_freq = hal::NearestFreq(CHOPPER_FREQ * 2), 
        .count_mode = hal::TimerCountMode::CenterAlignedUpTrig
    })  .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,

            hal::TimerChannelSelection::CH1N,
            hal::TimerChannelSelection::CH2N,
            hal::TimerChannelSelection::CH3N,
        }).unwrap()
        ;


    auto & pwm_u_ = timer.oc<1>(); 
    auto & pwm_v_ = timer.oc<2>(); 
    auto & pwm_w_ = timer.oc<3>(); 

    timer.ocn<1>().init(Default); 
    timer.ocn<2>().init(Default); 
    timer.ocn<3>().init(Default); 
    
    pwm_u_.init(Default);
    pwm_v_.init(Default);
    pwm_w_.init(Default);
    timer.oc<4>().init(Default);
    timer.oc<4>().cvr() = timer.arr() - 1;

    timer.oc<4>().enable_output(EN);

    auto uvw_pwmgen_ = UvwPwmgen(&pwm_u_, &pwm_v_, &pwm_w_);

    // #endregion 初始化定时器

    // #region 配置编码器
    
    auto mag_encoder_cs_pin_ = hal::PB<12>();
    mag_encoder_cs_pin_.outpp();
    auto & spi = hal::spi2;

    spi.init({
        .remap = hal::SpiRemap::_0,
        .baudrate = hal::NearestFreq(18_MHz)
    });

    auto mag_encoder_ = MotorProfile::MagEncoder{
        &spi,
        spi.allocate_cs_pin(&mag_encoder_cs_pin_)
            .unwrap()
    };


    // #endregion

    // #region 初始化DRV8323


    auto drv8323_en_pin_ = hal::PA<11>();
    auto drv8323_slp_pin_ = hal::PA<12>();
    auto drv8323_nfault_pin_ = hal::PA<6>();
    drv8323_nfault_pin_.inpu();

    drv8323_en_pin_.outpp(LOW);
    drv8323_slp_pin_.outpp(LOW);

    auto drv8323_mode_pin_      = hal::PB<4>();
    auto drv8323_vds_pin_       = hal::PB<3>();
    auto drv8323_idrive_pin_    = hal::PB<5>();
    auto drv8323_gain_pin_      = hal::PA<15>();


    drv8323_mode_pin_.outpp(LOW);      //6x pwm
    // drv8323_mode_pin_.outpp(HIGH);    //independent

    // drv8323_gain_pin_.outpp(LOW);
    // drv8323_gain_pin_.outpp(LOW);
    // drv8323_gain_pin_.inpd();//10x
    drv8323_gain_pin_.inflt();//20x
    // drv8323_gain_pin_.outpp(HIGH);//40x


    drv8323_idrive_pin_.outpp(HIGH);//Sink 2A / Source1A
    // drv8323_idrive_pin_.inflt();
    // drv8323_idrive_pin_.outpp(LOW);


    drv8323_vds_pin_.outpp(LOW); //10A保护
    // drv8323_vds_pin_.outpp(HIGH); //dangerous no ocp protect!!!!


    // #endregion 初始化ADC

    // #region 初始化ADC
    static constexpr auto VOLTAGE_TO_CURRENT_RATIO = iq16(3.3 * 0.5);
    auto soa_ = hal::ScaledAnalogInput(hal::adc1.inj<1>(), 
        Rescaler<iq16>::from_anti_offset(0.497_r) * Rescaler<iq16>::from_scale(VOLTAGE_TO_CURRENT_RATIO ));
    auto sob_ = hal::ScaledAnalogInput(hal::adc1.inj<2>(), 
        Rescaler<iq16>::from_anti_offset(0.497_r) * Rescaler<iq16>::from_scale(VOLTAGE_TO_CURRENT_RATIO ));
    auto soc_ = hal::ScaledAnalogInput(hal::adc1.inj<3>(), 
        Rescaler<iq16>::from_anti_offset(0.497_r)* Rescaler<iq16>::from_scale(VOLTAGE_TO_CURRENT_RATIO ));

    init_adc();
    // #endregion 

    // #region 初始化LED
    auto led_blue_pin_ = hal::PC<13>();
    auto led_red_pin_ = hal::PC<14>();
    auto led_green_pin_ = hal::PC<15>();

    led_red_pin_.outpp(); 
    led_blue_pin_.outpp(); 
    led_green_pin_.outpp();

    [[maybe_unused]] auto poll_blink_service = [&]{

        led_red_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 200) > 100);
        led_blue_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
        led_green_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 800) > 400);
    };

    [[maybe_unused]] auto toggle_red_led = [&]{
        led_red_pin_.toggle();
    };

    [[maybe_unused]] auto toggle_blue_led = [&]{
        led_blue_pin_.toggle();
    };

    // #endregion 
    
    Angular<uq16> openloop_elec_angle_ = Zero;
    Angular<uq16> sensored_elec_angle_ = Zero;
    Angular<uq16> encoder_lap_angle_ = Zero;
    Angular<iq16> encoder_multilap_angle_ = Zero;

    UvwCoord<iq20> uvw_curr_ = Zero;
    DqCoord<iq20> dq_curr_ = Zero;
    DqCoord<iq20> dq_volt_ = Zero;
    AlphaBetaCoord<iq20> alphabeta_curr_ = Zero;
    AlphaBetaCoord<iq20> alphabeta_volt_ = Zero;

    Microseconds exe_us_ = 0us;

    Leso leso = Leso{
        MotorProfile::leso_coeffs
    };

    Leso::State leso_state_var_ = Zero;

    static constexpr auto controller_coeff = current_regulator_cfg.try_to_coeff().unwrap();
    // PANIC{controller_coeff};

    auto d_pi_ctrl_ = controller_coeff.to_pi_controller();
    auto q_pi_ctrl_ = controller_coeff.to_pi_controller();

    [[maybe_unused]] auto flux_sensorless_ob = dsp::motor_ctl::NonlinearFluxObserver{
        dsp::motor_ctl::NonlinearFluxObserver::Config{
            .fs = FOC_FREQ,
            .phase_inductance = MotorProfile::PHASE_INDUCTANCE,
            .phase_resistance = MotorProfile::PHASE_RESISTANCE,

            // .observer_gain = 0.16_iq20, // [rad/s]
            .observer_gain = 0.1201_iq20, // [rad/s]
            // .pm_flux_linkage = 0.000017_iq20, // [V / (rad/s)]
            .pm_flux_linkage = 0.00084_iq20, // [V / (rad/s)]
        }
    };

    [[maybe_unused]] auto lbg_sensorless_ob = dsp::motor_ctl::LuenbergerObserver{
        dsp::motor_ctl::LuenbergerObserver::Config{
            .fs = FOC_FREQ,
            .phase_inductance = MotorProfile::PHASE_INDUCTANCE,
            .phase_resistance = MotorProfile::PHASE_RESISTANCE
        }
    };

    [[maybe_unused]] auto smo_sensorless_ob = dsp::motor_ctl::SlideModeObserver{
        dsp::motor_ctl::SlideModeObserver::Config{
            .f_para = 0.84_r,
            .g_para = 0.015_r,
            .kslide = 1.22_r,   
            .kslf = 0.6_r,   
        }
    };

    using Nltd2o = NonlinearTrackingDifferentiator<iq16, 2>;

    #if 0
    static constexpr auto coeffs = Nltd2o::Config{
        .fs = FOC_FREQ,
        // .r = 30.5_q24,
        // .h = 2.5_q24
        // .r = 252.5_iq10,
        // .r = 152.5_iq10,
        .r = 202.5_iq10,
        .h = 0.01_iq10,
        .x2_limit = 240
    }.try_into_coeffs().unwrap();
    #else
    static constexpr auto coeffs = Nltd2o::Config{
        .fs = FOC_FREQ,
        // .r = 30.5_q24,
        // .h = 2.5_q24
        // .r = 252.5_iq10,
        // .r = 152.5_iq10,
        // .r = 12.5_iq10,
        // .r = 37.5_iq10,
        .r = 587.5_iq10,
        .h = 0.01_iq10,
        .x2_limit = 40
    }.try_into_coeffs().unwrap();

    #endif

    static Nltd2o command_shaper_{
        coeffs
    };

    SecondOrderState<iq16> track_ref_;
    SecondOrderState<iq16> feedback_state_var_;

    using Ltd2o = dsp::adrc::LinearTrackingDifferentiator<iq16, 2>;
    static constexpr auto feedback_coeffs = Ltd2o::Config{
        .fs = FOC_FREQ, .r = 500
    }.try_into_coeffs().unwrap();

    // PANIC{feedback_coeffs};

    [[maybe_unused]] Ltd2o feedback_differ_{
        feedback_coeffs
    };

    UvwCoord<iq16> uvw_dutycycle_ = Zero;
    iq20 busbar_current_unfilted_ = Zero;
    iq20 busbar_current_ = Zero;
    auto ctrl_isr = [&]{
        [[maybe_unused]] const auto now_secs = clock::time();

        //#region 电流传感
        const auto uvw_curr = UvwCoord<iq20>{
            .u = soa_.get_value(),
            .v = sob_.get_value(),
            .w = soc_.get_value(),
        };

        const auto alphabeta_curr = AlphaBetaCoord<iq20>::from_uvw(uvw_curr_);
        //#endregion

        //#region 位置提取
        // const auto openloop_manchine_angle = Angular<iq16>::from_turns(0.2_iq16 * now_secs);
        const auto openloop_manchine_angle = Angular<iq16>::from_turns(0.1_iq16 * now_secs);
        // const auto openloop_manchine_angle = Angular<iq16>::from_turns(1.2_r * now_secs);
        // const auto openloop_manchine_angle = Angular<iq16>::from_turns(math::sinpu(0.2_r * now_secs));
        const auto openloop_elec_angle = openloop_manchine_angle * MotorProfile::POLE_PAIRS;

        mag_encoder_.update().examine();
        const auto next_encoder_lap_angle = mag_encoder_.read_lap_angle().examine().cast_inner<uq16>();

        const auto diff_angle = (next_encoder_lap_angle.cast_inner<iq16>() 
            - encoder_lap_angle_.cast_inner<iq16>()).signed_normalized();

        encoder_lap_angle_ = next_encoder_lap_angle;
        encoder_multilap_angle_ = encoder_multilap_angle_ + diff_angle;
        feedback_state_var_ = feedback_differ_.update(feedback_state_var_, {encoder_multilap_angle_.to_turns(), 0});
        
        const auto sensored_elec_angle = ((next_encoder_lap_angle * MotorProfile::POLE_PAIRS)
                + MotorProfile::SENSORED_ELEC_ANGLE_BASE).unsigned_normalized(); 


        #if 1
        // const auto elec_angle = openloop_elec_angle;
        const auto elec_angle = sensored_elec_angle;
        #else
        // const auto elec_angle = Angular<iq16>(flux_sensorless_ob.angle()) - 10_deg;
        // const auto elec_angle = Angular<iq16>(flux_sensorless_ob.angle()) - 20_deg;
        // const auto elec_angle = Angular<iq16>(flux_sensorless_ob.angle()) - 40_deg;
        // const auto elec_angle = Angular<iq16>(flux_sensorless_ob.angle() - 90_deg);
        // const auto elec_angle = Angular<iq16>(flux_sensorless_ob.angle() + 80_deg);
        // const auto elec_angle = Angular<iq16>(flux_sensorless_ob.angle()) - 40_deg;
        // const auto elec_angle = Angular<iq16>(flux_sensorless_ob.angle() + 180_deg + 30_deg);
        // const auto elec_angle = Angular<iq16>(flux_sensorless_ob.angle() + 50_deg);
        // const auto elec_angle = Angular<iq16>(flux_sensorless_ob.angle() - 22_deg);
        // const auto elec_angle = Angular<iq16>(flux_sensorless_ob.angle() - 135_deg);
        // const auto elec_angle = Angular<iq16>(lbg_sensorless_ob.angle() + 30_deg);
        // const auto elec_angle = Angular<iq16>(smo_sensorless_ob.angle() + 90_deg);
        // const auto elec_angle = Angular<iq16>(smo_sensorless_ob.angle() + 90_deg);
        // const auto elec_angle = Angular<iq16>(smo_sensorless_ob.angle() + 90_deg);
        #endif

        const auto elec_rotation = Rotation2<iq16>::from_angle(elec_angle);
        const auto elec_omega = feedback_state_var_.x2 * MotorProfile::POLE_PAIRS;
        //#endregion

        //#region 位速合成力矩
        const auto [position_cmd, speed_cmd] = [&]{

            enum class ExamplePattern{
                Sine,
                Saw,
                Levels
            };

            // static constexpr auto example_pattern = ExamplePattern::Saw;
            static constexpr auto example_pattern = ExamplePattern::Levels;

            const auto [x1_cmd, x2_cmd] = [&] -> std::tuple<iq16, iq16>{
                if constexpr(example_pattern == ExamplePattern::Sine){
                    constexpr auto omega = 16_iq16;
                    constexpr auto side_amplitude = 0.4_iq16;

                    const auto [s,c] = math::sincos(omega * now_secs);
                    return {
                        side_amplitude * iq16(s),
                        side_amplitude * omega * iq16(c)
                    };
                }else if constexpr(example_pattern == ExamplePattern::Saw){
                    // const auto [s,c] = math::sincos(omega * now_secs);

                    constexpr auto freq = 1.6_iq16;
                    constexpr auto amplitude = 0.8_iq16;
                    constexpr auto slew_rate = amplitude * freq;
                    return {math::frac(now_secs * freq) * amplitude, slew_rate};
                }else if constexpr(example_pattern == ExamplePattern::Levels){
                    constexpr auto freq = 0.3_iq16;
                    constexpr size_t num_steps = 6;
                    constexpr auto half_amplitude = 0.4_iq16;
                    constexpr auto step = half_amplitude * 2/ num_steps;
                    const auto s = iq16(math::sinpu(now_secs * freq));
                    return {(math::floor(s * (num_steps / 2)) * step), 0};
                }
            }();
            // command_shaper_.update(10 + 12 * sign(iq16(math::sinpu(now_secs * 0.5_r))));
            // const auto s = iq16(math::sinpu(now_secs * 0.7_r));
            // const auto s = iq16(math::sinpu(now_secs * 0.16_r));
            // command_shaper_.update(100 + 6 * (int(s * 8) / 8));
            // track_ref_ = command_shaper_.update(track_ref_, );
            track_ref_ = command_shaper_.update(track_ref_, {
                x1_cmd, 
                x2_cmd
            });
            // track_ref_ = command_shaper_.update(track_ref_, {now_secs * 15, 15});
            // track_ref_ = command_shaper_.update(track_ref_, {
            //     1_iq16 * iq16(sin(now_secs / 100)),
            //     0.01_iq16 *  iq16(cos(now_secs / 100))});
                // 1_iq16 * iq16(sin(now_secs)),
                // 1_iq16 *  iq16(cos(now_secs))});
                // _iq16 * iq16(sin(now_secs)),
                // 1_iq16 *  iq16(cos(now_secs))});
            return std::make_tuple(
                iq16::from_bits(track_ref_.x1.to_bits() >> 16),
                track_ref_.x2
            );

            // return std::make_tuple<iq16, iq16>(
            //     amplitude * int(omega * now_secs),
            //     0
            // );
        }();


        [[maybe_unused]] const iq20 torque_cmd = [&]{ 
            const auto kp = MotorProfile::KP;
            const auto kd = MotorProfile::KD;

            // const iq16 e1 = position_cmd - pos_filter_.accumulated_angle().to_turns();
            // const iq16 e2 = speed_cmd - pos_filter_.speed();
            const iq16 e1 = CLAMP2(position_cmd - math::fixed_downcast<16>(feedback_state_var_.x1), 100);
            const iq16 e2 = CLAMP2(speed_cmd - feedback_state_var_.x2, 1000);

            return CLAMP2((kp * e1) + (kd * e2), 1);
        }();

        //#endregion 

        //#region 力矩转电流

        [[maybe_unused]] static constexpr iq20 TORQUE_2_CURR_RATIO = 1_iq20;
        [[maybe_unused]] static constexpr iq20 CURRENT_LIMIT = 1.2_iq16;

        // const iq20 current_cmd = CLAMP2((torque_cmd - math::fixed_downcast<16>(leso_state_var_.x2) / 1000) * TORQUE_2_CURR_RATIO, CURRENT_LIMIT);
        // const iq20 current_cmd = CLAMP2((torque_cmd) * TORQUE_2_CURR_RATIO, CURRENT_LIMIT);
        
        // const iq20 current_cmd = 0.14_iq20;
        // const iq20 current_cmd = 0.1_iq20 * iq16(math::sin(now_secs));
        const iq20 current_cmd = CLAMP2((torque_cmd) * TORQUE_2_CURR_RATIO, CURRENT_LIMIT);
        // const iq20 current_cmd = 0.07_iq20 * iq16(sin(now_secs));
        // const iq20 current_cmd = 0.07_iq20 * -1;
        //#endregion

        const auto dq_curr = alphabeta_curr.to_dq(elec_rotation);

        [[maybe_unused]] auto generate_dq_volt_by_pi_ctrl = [&]{
            const iq20 dest_d_curr = 0;
            const iq20 dest_q_curr = current_cmd;
            // return DqCoord<iq20>{
            //     .d = d_pi_ctrl_(dest_d_curr - dq_curr.d),
            //     .q = q_pi_ctrl_(dest_q_curr - dq_curr.q)
            // };
            return DqCoord<iq20>{
                .d = d_pi_ctrl_(dest_d_curr - dq_curr.d) - MotorProfile::PHASE_INDUCTANCE * dq_curr.q * elec_omega,
                .q = q_pi_ctrl_(dest_q_curr - dq_curr.q) + MotorProfile::PHASE_INDUCTANCE * dq_curr.d * elec_omega
            };
        };

        #if 1
        [[maybe_unused]] auto generate_dq_volt_by_constant_voltage = [&]{
            auto dq_volt = dq_volt_;
            dq_volt.d = 0_r;
            dq_volt.q = 0.4_r;
            return dq_volt;
        };
        #endif



        // const auto dq_volt = (generate_dq_volt_by_hfi()).clamp(MODU_VOLT_LIMIT);
        const auto dq_volt = (generate_dq_volt_by_pi_ctrl()).clamp(MotorProfile::MODU_VOLT_LIMIT);
        // const auto dq_volt = generate_dq_volt_by_constant_voltage().clamp(MODU_VOLT_LIMIT);

        #if 0
        [[maybe_unused]] auto generate_alpha_beta_volt_by_hfi = [&]{
            static constexpr size_t HFI_MAX_STEPS = 4;
            static constexpr auto HFI_VOLT_LIMIT = 1.0_r;
            static size_t hfi_step = 0;
            hfi_step = (hfi_step + 1) % HFI_MAX_STEPS;
            
            return AlphaBetaCoord<iq20>{
                .alpha = HFI_VOLT_LIMIT * math::sinpu(iq16(hfi_step) / (HFI_MAX_STEPS)),
                // .alpha = HFI_VOLT_LIMIT,
                .beta = 0_r,
            };
        };
        #endif


        const auto alphabeta_volt = dq_volt.to_alphabeta(elec_rotation);
        // const auto alphabeta_volt = AlphaBetaCoord<iq20>::ZERO;
        // const auto alphabeta_volt = generate_alpha_beta_volt_by_hfi().clamp(MODU_VOLT_LIMIT);

        // flux_sensorless_ob.update(alphabeta_volt, alphabeta_curr);
        // smo_sensorless_ob.update(alphabeta_volt, alphabeta_curr);

        const auto uvw_dutycycle = SVM(
            AlphaBetaCoord<iq16>{
                .alpha = alphabeta_volt.alpha, 
                .beta = alphabeta_volt.beta
            } * INV_BUS_VOLT
        );

        

        // leso_state_var_ = leso.iterate(leso_state_var_, feedback_state_var_.x1, current_cmd);
        leso_state_var_ = leso.iterate(leso_state_var_, feedback_state_var_.x2, current_cmd);
        // leso_state_var_ = leso.iterate(leso_state_var_, math::fixed_downcast<16>(feedback_state_var_.x1), current_cmd);
        busbar_current_unfilted_ = UvwCoord<iq20>(uvw_dutycycle.u, uvw_dutycycle.v, uvw_dutycycle.w).dot(uvw_curr);
        busbar_current_ = lpf_exprimetal(busbar_current_, busbar_current_unfilted_);
        uvw_pwmgen_.set_dutycycle(uvw_dutycycle);
        uvw_dutycycle_ = uvw_dutycycle;
        uvw_curr_ = uvw_curr;
        alphabeta_curr_ = alphabeta_curr;
        dq_curr_ = dq_curr;
        dq_volt_ = dq_volt;
        alphabeta_volt_ = alphabeta_volt;
        openloop_elec_angle_ = openloop_elec_angle;
        sensored_elec_angle_ = sensored_elec_angle;
    };
    auto jeoc_isr = [&]{ 
        const auto begin_us = clock::micros();
        ctrl_isr();
        const auto end_us = clock::micros();
        exe_us_ = end_us - begin_us;
    };

    hal::adc1.register_nvic({0,0}, EN);
    hal::adc1.enable_interrupt<hal::AdcIT::JEOC>(EN);
    hal::adc1.set_event_handler(
        [&](const hal::AdcEvent ev){
            switch(ev){
            case hal::AdcEvent::EndOfInjectedConversion:{
                jeoc_isr();
                break;
            }
            default: break;
            }
        }
    );

    [[maybe_unused]] auto repl_service_poller = [&]{
        static robots::ReplServer repl_server{&DBG_UART, &DBG_UART};

        static const auto list = rpc::make_list(
            "list",

            rpc::make_function("errn", [&](int32_t a, int32_t b){ 
                DEBUG_PRINTLN(a,b);
            }),
            rpc::make_function("errn2", [&](int32_t a, int32_t b){ 
                DEBUG_PRINTLN(a,b);
            })

        );

        repl_server.invoke(list);
    };

    DBG_UART.set_event_handler([&](const hal::UartEvent & event){
        switch(event.kind()){
            case hal::UartEvent::TxIdle:{
                // __builtin_trap();
                poll_blink_service();
                // toggle_blue_led();
                break;
            }
            case hal::UartEvent::RxIdle:{
                // __builtin_trap();
                // poll_blink_service();
                toggle_blue_led();
                break;
            }
            default:{
                break;
            }
        }
    });


    timer.start();

    while(true){
        // repl_service_poller();
            
        static auto report_timer = async::RepeatTimer::from_duration(5ms);
        report_timer.invoke_if([&]{
            if(false)DEBUG_PRINTLN(
                pwm_u_.cvr(),
                pwm_v_.cvr(),
                pwm_w_.cvr()
            );
            if(false){
            // if(false){
                DEBUG_PRINTLN(
                    busbar_current_unfilted_,
                    busbar_current_,
                    // sensored_elec_angle_.to_turns()

                    uvw_curr_

                    // uvw_curr_
                    // hal::adc1.inj<1>().get_perunit(),
                    // hal::adc1.inj<2>().get_perunit()
                    // soa_.get_value(),
                    // sob_.get_value()
                );
            }
            if(false){
                const auto now_secs = clock::time();
                const auto t = frac(now_secs * 2);
                const auto [s,c] = math::sincospu(t);

                DEBUG_PRINTLN(
                    // math::sin(now_secs),
                    // hal::usart2.available()
                    // SVM_unified(AlphaBetaCoord<iq16>{iq16(c),iq16(s)}),
                    // SVM_reference(AlphaBetaCoord<iq16>{iq16(s),iq16(c)}),
                    SVM(AlphaBetaCoord<iq16>{iq16(c) * iq16(0.866),iq16(s) * iq16(0.866)}),
                    t
                    // u1.count(),
                    // u2.count(),
                    // i
                    // 0
                );
                // DEBUG_PRINTLN(1);
                // const char str[] = "h\r\n";
                // hal::usart2.write_chars(str, sizeof(str) -1);
                // clock::delay(200ms);
            }
            if(true) DEBUG_PRINTLN(
            // if(true) DEBUG_PRINTLN(
                // alphabeta_curr_,
                // alphabeta_volt_,
                // alphabeta_volt_.beta / alphabeta_curr_.beta,
                // dq_curr_,
                // dq_volt_,
                // alphabeta_volt_.beta / alphabeta_curr_.beta,
                // q_pi_ctrl_.err_sum_,
                // q_pi_ctrl_.kp_
                // q_pi_ctrl_.err_sum_max_
                // lbg_sensorless_ob.angle().to_turns(),
                // hal::adc1.inj<1>().get_voltage(),
                
                // iq16(lap_angle.to_turns()) * POLE_PAIRS,
                // sensored_elec_angle_.to_turns(),
                // openloop_elec_angle_.to_turns(),
                iq16::from_bits(track_ref_.x1.to_bits() >> 16),
                // track_ref_.x2,
                iq16::from_bits(feedback_state_var_.x1.to_bits() >> 16),
                feedback_state_var_.x2,
                // feedback_state_var_.x2,
                // leso_state_var_.x2,
                // encoder_lap_angle_.to_turns(),
                // sensored_elec_angle_.to_turns(),
                encoder_multilap_angle_.to_turns(),
                busbar_current_unfilted_,
                busbar_current_,
                0
                // encoder_multilap_angle_.to_turns(),
                // leso_state_var_.x2,
                // sensored_elec_angle_.to_turns(),
                // encoder_multilap_angle_.to_turns(),
                // 0
                // flux_sensorless_ob.angle().to_turns()
                // flux_sensorless_ob.V_alphabeta_last_
                // smo_sensorless_ob.angle().to_turns(),
                // exe_us_.count(),

                // flux_sensorless_ob.angle().to_turns(),
                // lbg_sensorless_ob.angle().to_turns(),
                // smo_sensorless_ob.angle().to_turns(),
                // uint32_t(exe_us_.count())

                // openloop_elec_angle_.normalized().to_turns()
                // bool(drv8323_nfault_pin_.read() == LOW),


                // pwm_u_.cvr(),
                // pwm_v_.cvr(),
                // pwm_w_.cvr(),

                // pwm_u_.get_dutycycle(),
                // pwm_v_.get_dutycycle(),
                // pwm_w_.get_dutycycle(),
            );
        });

        poll_blink_service();
        // toggle_red_led();
        // repl_service_poller();
        // clock::delay(2ms);
    }

}