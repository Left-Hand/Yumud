#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/polymorphism/traits.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"

#include "algo/interpolation/cubic.hpp"

#include "robots/rpc/rpc.hpp"
#include "src/testbench/tb.h"

#include "robots/rpc/arg_parser.hpp"

#include "app/stepper/ctrl.hpp"

#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/GateDriver/MP6540/mp6540.hpp"
#include "digipw/SVPWM/svpwm.hpp"
#include "digipw/SVPWM/svpwm3.hpp"
#include "drivers/GateDriver/DRV8301/DRV8301.hpp"

#include "dsp/observer/smo/SmoObserver.hpp"
#include "dsp/observer/lbg/RolbgObserver.hpp"
#include "dsp/observer/nonlinear/NonlinearObserver.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "dsp/controller/pi_ctrl.hpp"
#include "core/polymorphism/reflect.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "CurrentSensor.hpp"


using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::foc;
using namespace ymd::digipw;
using namespace ymd::dsp;
using namespace ymd::intp;

scexpr uint32_t CHOPPER_FREQ = 50000;
scexpr uint32_t FOC_FREQ = CHOPPER_FREQ;



class CurrentSensor{
protected:
    hal::AnalogInIntf & u_sense_;
    hal::AnalogInIntf & v_sense_;
    hal::AnalogInIntf & w_sense_;

    UvwCurrent uvw_bias_;
    UvwCurrent uvw_raw_;
    UvwCurrent uvw_curr_;
    real_t mid_curr_;
    AbCurrent ab_curr_;
    DqCurrent dq_curr_;

    dsp::ButterLowpassFilter<q16, 2> mid_filter_ = {{
        .fc = 400,
        .fs = FOC_FREQ
    }};


    void capture(){
        uvw_raw_ = {
            real_t(u_sense_),
            real_t(v_sense_),
            real_t(w_sense_)
        };
        // uvw_raw_ = {
        //     real_t(0),
        //     real_t(0),
        //     real_t(0)
        // };

        mid_filter_.update((uvw_raw_.u + uvw_raw_.v + uvw_raw_.w) * q16(1.0/3));

        mid_curr_ = mid_filter_.get();
        uvw_curr_[0] = (uvw_raw_.u - mid_curr_ - uvw_bias_.u);
        uvw_curr_[1] = (uvw_raw_.v - mid_curr_ - uvw_bias_.v);
        uvw_curr_[2] = (uvw_raw_.w - mid_curr_ - uvw_bias_.w);

        ab_curr_ = AbCurrent::from_uvw(uvw_curr_);
    }

public:
    CurrentSensor(
        hal::AnalogInIntf & u_sense,
        hal::AnalogInIntf & v_sense, 
        hal::AnalogInIntf & w_sense
    ): 
        u_sense_(u_sense),
        v_sense_(v_sense), 
        w_sense_(w_sense){
            reset();
        }

    void reset(){
        uvw_curr_ = {0, 0, 0};
        uvw_bias_ = {0, 0, 0};
        ab_curr_ = {0, 0};
        dq_curr_ = {0, 0};
    }

    void update(const real_t rad){
        
            
        capture();

        dq_curr_ = DqCurrent::from_ab(ab_curr_, rad);
    }


    const auto &  raw()const {return uvw_raw_;}
    const auto &  mid() const {return mid_curr_;}
    const auto &  uvw()const{return uvw_curr_;}
    // auto uvw(){return uvw_curr_;}
    const auto & ab()const{return ab_curr_;}
    // auto ab(){return ab_curr_;}
    const auto & dq()const{return dq_curr_;}
    // auto dq(){return dq_curr_;}
};


template<size_t N>
__fast_inline iq_t<16> LPFN(const iq_t<16> x, const iq_t<16> y){
    constexpr size_t sc = ((1 << N) - 1);
    return (x * sc + y) >> N;
}


__inline constexpr real_t degrees(const real_t deg){
    return deg * real_t(TAU / 180);
}

TRAIT_STRUCT(SensorlessObserverTrait,
    TRAIT_METHOD(void, reset),
	TRAIT_METHOD(void, update, iq_t<16>, iq_t<16>, iq_t<16>, iq_t<16>),
    TRAIT_METHOD(iq_t<16>, theta)
)



void init_adc(hal::AdcPrimary & adc){

    using hal::AdcChannelIndex;
    using hal::AdcSampleCycles;

    adc.init(
        {
            {AdcChannelIndex::VREF, AdcSampleCycles::T28_5}
        },{
            // {AdcChannelIndex::CH5, AdcSampleCycles::T28_5},
            // {AdcChannelIndex::CH4, AdcSampleCycles::T28_5},
            // {AdcChannelIndex::CH1, AdcSampleCycles::T28_5},

            // {AdcChannelIndex::CH5, AdcSampleCycles::T7_5},
            // {AdcChannelIndex::CH4, AdcSampleCycles::T7_5},
            // {AdcChannelIndex::CH1, AdcSampleCycles::T7_5},
            // {AdcChannelIndex::VREF, AdcSampleCycles::T7_5},

            {AdcChannelIndex::CH5, AdcSampleCycles::T13_5},
            {AdcChannelIndex::CH4, AdcSampleCycles::T13_5},
            {AdcChannelIndex::CH1, AdcSampleCycles::T13_5},
            {AdcChannelIndex::VREF, AdcSampleCycles::T7_5},
            // {AdcChannelIndex::TEMP, AdcSampleCycles::T7_5},
            // AdcChannelConfig{AdcChannelIndex::CH1, AdcCycles::T7_5},
            // AdcChannelConfig{AdcChannelIndex::CH4, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelIndex::CH5, AdcCycles::T28_5},
            // AdcChannelConfig{AdcChannelIndex::CH1, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelIndex::CH4, AdcCycles::T41_5},
            // AdcChannelConfig{AdcChannelIndex::CH5, AdcCycles::T41_5},
        }, {}
    );

    // adc.setTrigger(AdcOnChip::RegularTrigger::SW, AdcOnChip::InjectedTrigger::T1TRGO);
    adc.set_injected_trigger(hal::AdcInjectedTrigger::T1CC4);
    // adc.enableContinous();
    adc.enable_auto_inject(DISEN);
}


class SensorlessEncoder:public EncoderIntf{
protected:
    SensorlessObserverTrait & ob_;
public:
    SensorlessEncoder(
        SensorlessObserverTrait & ob
    ):
        ob_(ob){;}
};


enum class MotorId:uint8_t{
    Old,
    New
};


class CurrentBiasCalibrater{
public:
    struct Config{
        uint32_t period_ticks;
        uint32_t fc;
        uint32_t fs;
    };

    using Lpf = LowpassFilter<iq_t<16>>;
    using Lpfs = std::array<Lpf, 3>;

    Lpfs lpfs_ = {};

protected:
    uint32_t period_ticks_;
    uint32_t elapsed_ticks_;
    uint32_t fs_;

    // real_t last_midp_curr_ = 0;
public:
    CurrentBiasCalibrater(const Config & config){
        reconf(config);
        reset();
    }

    void reconf(const Config & config){
        period_ticks_ = config.period_ticks;
        fs_ = config.fs;

        // const auto alpha = Lpf::solve_alpha(config.fc, config.fs);
        lpfs_[0].reconf({config.fc, config.fs});
        lpfs_[1].reconf({config.fc, config.fs});
        lpfs_[2].reconf({config.fc, config.fs});

    }

    void reset(){
        elapsed_ticks_ = 0;
        for(auto & lpf : lpfs_){
            lpf.reset();
        }
    }

    void update(const UvwCurrent & uvw, const real_t mid_point){
        lpfs_[0].update(uvw.u);
        lpfs_[1].update(uvw.v);
        lpfs_[2].update(uvw.w);
        elapsed_ticks_ ++;

        // constexpr auto stable_curr_slewrate = 10.0_r;
        // constexpr auto stable_threshold = stable_curr_slewrate / FOC_FREQ;

        // const auto mid_point_diff = ABS(mid_point - last_midp_curr_);
        // last_midp_curr_ = mid_point;

        // curr_stable_checker.update(mid_point_diff < stable_threshold);
    }

    bool is_done(){
        return elapsed_ticks_ >= period_ticks_;
    }

    UvwCurrent result() const{
        return {
            lpfs_[0].get(),
            lpfs_[1].get(),
            lpfs_[2].get(),
        };
    }
};

class CalibraterOrchestor{

};

__no_inline void init_opa(){
    hal::opa1.init<1,1,1>();
}

struct ElecradCompensator{
    q16 base;
    uint32_t pole_pairs;

    constexpr q16 operator ()(const q16 lap_position) const {
        return (frac(frac(lap_position + base) * pole_pairs) * real_t(TAU));
    }
};

void bldc_main(){
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

    DBG_UART.init({576000});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets();
    clock::delay(200ms);

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

    MA730 ma730{
        &spi,
        spi.attach_next_cs(&hal::portA[15])
            .unwrap()
    };
    ma730.init().unwrap();

    can.init({hal::CanBaudrate::_1M});

    const auto motor_id = []{
        const auto chip_id_crc = sys::chip::get_chip_id_crc();
        switch(chip_id_crc){
            case 207097585:
                return MotorId::New;
            default:
                return MotorId::Old;
        }
    }();

    

    // for(size_t i = 0; i < 1000; ++i) {
    //     bmi.update();
    //     // auto [x,y,z] = bmi.read_acc();
    //     auto [x,y,z] = bmi.read_gyr();
    //     DEBUG_PRINTLN(x,y,z);
    //     clock::delay(2ms);
    //     // DEBUGGER << std::endl;
    // }


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
    

    auto & u_sense = mp6540.ch(1);
    auto & v_sense = mp6540.ch(2);
    auto & w_sense = mp6540.ch(3);
    

    // init_opa();
    init_adc(adc);
    [[maybe_unused]] real_t targ_spd_ = 0;

    // UvwCurrent uvw_curr = {0,0,0};
    real_t est_rad;

    CurrentSensor curr_sens = {u_sense, v_sense, w_sense};
    // CurrentSensor curr_sens = {adc.inj(1), adc.inj(2), adc.inj(3)};

    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    hal::portA[7].inana();


    en_gpio.set();
    nslp_gpio.set();
    // uint32_t dt;

    // std::array<real_t, 2> ab_volt;

    // scexpr real_t r_ohms = 7.1_r;
    // scepxr real_t l_mh = 1.45_r;

    // scexpr iq_t<16> pll_freq = iq_t<16>(0.2);
    [[maybe_unused]]
    LapPosPll pll = {
        {
            // .kp = real_t(2 * pll_freq),
            .kp = 470.17_r,
            // .ki = real_t(pll_freq * pll_freq),
            .ki = 20.3_r,
            .fc = FOC_FREQ
        }
    };


    real_t mg_meas_rad_;
    real_t sl_meas_rad_;



    AbVoltage ab_volt_;
    
    dsp::PositionSensor pos_sensor_{
        typename dsp::PositionSensor::Config{
            .r = 85,
            .fs = FOC_FREQ
        }
    };

    ElecradCompensator elecrad_comp_{
        .base = [&]{
            switch(motor_id){
                case MotorId::New:
                    return -0.12_r;
                case MotorId::Old:
                    return -0.25_r;
                default:
                    __builtin_unreachable();
            }
        }(),
        .pole_pairs = 7
    };

    real_t d_volt_ = 0;
    real_t q_volt_ = 0;
    real_t meas_rad_ = 0;
    [[maybe_unused]] auto cb_sensored = [&]{
        ma730.update().examine();
        const auto meas_lap = 1-ma730.get_lap_position().examine(); 
        pos_sensor_.update(meas_lap);


        const real_t meas_rad = elecrad_comp_(meas_lap);

        #define TEST_MODE_Q_SIN_CURR 1
        #define TEST_MODE_VOLT_POS_CTRL 2
        #define TEST_MODE_WEAK_MAG 3
        #define TEST_MODE_POS_SIN 4
        #define TEST_MODE_SQUARE_SWING 5
        // #define TEST_MODE TEST_MODE_Q_SIN_CURR
        #define TEST_MODE TEST_MODE_POS_SIN
        // #define TEST_MODE TEST_MODE_SQUARE_SWING


        #if (TEST_MODE == TEST_MODE_Q_SIN_CURR)
        const auto d_curr_cmd = 0.0_r;
        const auto d_volt = d_pi_ctrl.update(d_curr_cmd, dq_curr.d);
        const auto q_curr_cmd =  0.2_r * sin(clock::time());
        const auto q_volt = q_pi_ctrl.update(q_curr_cmd, dq_curr.q);
        #elif (TEST_MODE == TEST_MODE_VOLT_POS_CTRL)

        #elif (TEST_MODE == TEST_MODE_WEAK_MAG)
        const auto d_curr_cmd = 0.0_r;
        const auto d_volt = d_pi_ctrl.update(d_curr_cmd, dq_curr.d);
        const auto q_curr_cmd =  0.2_r * sin(clock::time());
        const auto q_volt = q_pi_ctrl.update(q_curr_cmd, dq_curr.q);

        // const auto d_volt = d_pi_ctrl.update(0.2_r, dq_curr.d);
        // const auto q_volt = q_pi_ctrl.update(-0.6_r, dq_curr.q);

        // const auto d_volt = d_pi_ctrl.update(0.0_r, dq_curr.d);
        // const auto d_volt = d_pi_ctrl.update((MAX(ab_volt.length() * 0.03_r - 0.2_r)), dq_curr.d);
        // const auto d_curr_cmd = (meas_spd > 10) ? -CLAMP(ab_volt.length() * 0.03_r - 0.2_r, 0.0_r, 0.7_r) : 0.0_r;
        #elif (TEST_MODE == TEST_MODE_POS_SIN)
        scexpr real_t omega = 3 * real_t(TAU);
        // scexpr real_t omega = 1;
        scexpr real_t amp = 0.5_r;
        const auto clock_time = clock::time();
        const auto [targ_pos, targ_spd] = ({
            std::make_tuple(
                amp * sin(omega * clock_time), amp * omega * cos(omega * clock_time)
                // 2 * clock_time, 2
            );
        });

        const auto meas_pos = pos_sensor_.position();
        const auto meas_spd = pos_sensor_.speed();

        const auto d_volt = 0;
        const auto q_volt = CLAMP2(
            9.58_r * (targ_pos - meas_pos) +
            0.38_r * (targ_spd - meas_spd)
            
        , 3.7_r);
        // const auto q_volt = 1.7_r;
        
        // const auto q_volt = q_pi_ctrl.update(q_curr_cmd, dq_curr.q);

        #elif (TEST_MODE == TEST_MODE_SQUARE_SWING)
        scexpr real_t omega = 1 * real_t(TAU);
        scexpr real_t amp = 0.5_r;

        const auto d_curr_cmd = 0.0_r;
        const auto d_volt = d_pi_ctrl.update(d_curr_cmd, dq_curr.d);
        const auto q_curr_cmd = SIGN_AS(amp, sin(omega * clock::time()));
        const auto q_volt = q_pi_ctrl.update(q_curr_cmd, dq_curr.q);
        #endif

        const auto ab_volt = DqVoltage{d_volt, q_volt}.to_ab(meas_rad);
        // ab_volt = DqVoltage{d_volt, q_volt}.to_ab(-0.0_r);
        svpwm.set_ab_volt(
            ab_volt[0], ab_volt[1]
            // amp * cos(omega * clock_time), amp * sin(omega * clock_time)
            // 3.0_r, 0.0_r
        );

        d_volt_ = d_volt;
        q_volt_ = q_volt;
        meas_rad_ = meas_rad;
    };

    adc.attach(hal::AdcIT::JEOC, {0,0}, 
        cb_sensored
    );

    while(true){
        [[maybe_unused]] const auto t = clock::time();
        [[maybe_unused]] const auto uvw_curr = curr_sens.uvw();
        [[maybe_unused]] const auto dq_curr = curr_sens.dq();
        [[maybe_unused]] const auto ab_curr = curr_sens.ab();
        // DEBUG_PRINTLN_IDLE(curr_sens.raw(), calibrater.result(), calibrater.is_done(), speed_measurer.result());
        DEBUG_PRINTLN_IDLE(
            // phase_res,
            // real_t(adc.inj(1)),
            // real_t(adc.inj(2)),
            // real_t(adc.inj(3)),
            // real_t(adc.inj(4)),
            // hfi_result * 10,
            // hfi_mid_result * 10,
            // phase_ind * 100,
            // lbg_ob.theta(),
            // s,c,
            // t,
            // frac(t),
            // sin(t * 10),
            // SVM(c,s),
            // real_t(pwm_u), 
            // real_t(pwm_v),
            // real_t(pwm_w),
            // bool(pwm_u.io()),
            // bool(pwm_v.io()),
            // bool(pwm_w.io()),
            // curr_sens.uvw(),
            
            // curr_sens.ab(),
            // curr_sens.dq(),
            // lbg_ob.e_alpha_,
            // lbg_ob.e_beta_,

            // square (lbg_ob.e_alpha_) + square(lbg_ob.e_beta_),
            // lbg_ob.theta(),
            // lbg_ob.e_alpha_,
            // lbg_ob.e_beta_,
            // nlr_ob.theta(),
            // fmod(mg_meas_rad, q16(TAU)),
            // timer.oc<1>().get_duty(),
            // timer.oc<2>().get_duty(),
            // timer.oc<3>().get_duty(),
            // nslp_gpio.read().to_bool(),
            // en_gpio.read().to_bool(),
            // dq_curr.d,
            // dq_curr.q,
            d_volt_,
            q_volt_,
            pos_sensor_.position(),
            pos_sensor_.speed(),
            meas_rad_,
            sys::chip::get_chip_id_crc()
            // TIM1_CH1_GPIO.read().to_bool(),
            // TIM1_CH2_GPIO.read().to_bool(),
            // TIM1_CH3_GPIO.read().to_bool()
            // exe_micros.count()
            // pll.theta(),
            // clock::micros() * 0.001_r
            // real_t(exe_micros)
            // curr_sens.mid()
        );
        // DEBUG_PRINTLN_IDLE(odo.getPosition(), iq_t<16>(speed_measurer.result()), sin(t), t);
        // if(false)

        ledr = BoolLevel::from((uint32_t(clock::millis().count()) % 200) > 100);
        ledb = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
        ledg = BoolLevel::from((uint32_t(clock::millis().count()) % 800) > 400);


    }
}