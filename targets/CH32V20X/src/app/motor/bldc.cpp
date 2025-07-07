#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/polymorphism/traits.hpp"
#include "core/polymorphism/reflect.hpp"
#include "core/sync/timer.hpp"

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


#include "digipw/SVPWM/svpwm.hpp"
#include "digipw/SVPWM/svpwm3.hpp"

#include "dsp/observer/smo/SmoObserver.hpp"
#include "dsp/observer/lbg/RolbgObserver.hpp"
#include "dsp/observer/nonlinear/NonlinearObserver.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "dsp/controller/pi_ctrl.hpp"

#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "CurrentSensor.hpp"

#include "types/vectors/quat/Quat.hpp"

#include "algo/interpolation/cubic.hpp"

#include "src/testbench/tb.h"

#include "robots/rpc/arg_parser.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/repl/repl_service.hpp"

#include "app/stepper/ctrl.hpp"

using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::foc;
using namespace ymd::digipw;
using namespace ymd::dsp;
using namespace ymd::intp;

scexpr uint32_t CHOPPER_FREQ = 25000;
scexpr uint32_t FOC_FREQ = CHOPPER_FREQ;

namespace ymd::dsp{
template<typename T>
struct ComplementaryFilter{
    struct Config{
        T kq;
        T ko;
        uint fs;
    };
    

    constexpr ComplementaryFilter(const Config & config){
        reconf(config);
        reset();
    }


    constexpr void reconf(const Config & cfg){
        kq_ = cfg.kq;
        kq_ = cfg.kq;
        dt_ = T(1) / cfg.fs;
    }

    constexpr T operator ()(const T rot, const T gyr){

        if(!inited_){
            rot_ = rot;
            rot_unfiltered_ = rot;
            inited_ = true;
        }else{
            rot_unfiltered_ += gyr * delta_t_;
            rot_unfiltered_ = kq_ * rot_ + (1-kq_) * rot;
            rot_ = ko_ * rot_ + (1-ko_) * rot_unfiltered_;
        }
    
        last_rot_ = rot;
        last_gyr_ = gyr;

        return rot_;
    }

    constexpr void reset(){
        rot_ = 0;
        rot_unfiltered_ = 0;
        last_rot_ = 0;
        last_gyr_ = 0;
        inited_ = false;
    }

    constexpr T get() const {
        return rot_;
    }

private:
    T kq_;
    T ko_;
    T dt_;
    T rot_;
    T rot_unfiltered_;
    T last_rot_;
    T last_gyr_;
    // T last_time;

    uint delta_t_;
    
    bool inited_;
};

}

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


enum class MotorRole:uint8_t{
    Roll,
    Pitch
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

struct HomePositionCompensator{
    q16 base;

    struct Input{
        const q16 position;
        const q16 lap_position;
    };

    constexpr q16 operator ()(const Input input) const {
        return input.lap_position - base;
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
    clock::delay(2ms);

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

    BMI160 bmi{
        &spi,
        spi.attach_next_cs(&hal::portA[0])
            .unwrap()
    };

    const auto motor_role = []{
        const auto chip_id_crc = sys::chip::get_chip_id_crc();
        switch(chip_id_crc){
            case 207097585:
                return MotorRole::Pitch;
            default:
                return MotorRole::Roll;
        }
    }();

    ma730.init().examine();

    
    bmi.init({
        .acc_odr = BMI160::AccOdr::_200Hz,
        .acc_fs = BMI160::AccFs::_16G,
        .gyr_odr = BMI160::GyrOdr::_200Hz,
        .gyr_fs = BMI160::GyrFs::_2000deg
    }).examine();

    // while(true){
    //     bmi.update().examine();
    //     const auto acc = bmi.read_acc().examine();
    //     const auto gyr = bmi.read_gyr().examine(); 
    //     const auto stat = uint8_t(bmi.get_pmu_mode(BMI160::PmuType::GYR).examine());
    //     const auto stat2 = uint8_t(bmi.get_pmu_mode(BMI160::PmuType::ACC).examine());
    //     DEBUG_PRINTLN(
    //         acc, 
    //         gyr, 
    //         stat,
    //         stat2
    //     );
    //     clock::delay(3ms);
    // }

    can.init({hal::CanBaudrate::_1M});

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


    real_t mg_meas_rad_;
    real_t sl_meas_rad_;
    real_t base_roll_ = 0;
    real_t base_omega_ = 0;



    AbVoltage ab_volt_;
    
    dsp::PositionSensor pos_sensor_{
        typename dsp::PositionSensor::Config{
            .r = 85,
            .fs = FOC_FREQ
        }
    };

    // pos_sensor_.set_base_position(0.2_r);
    // pos_sensor_.set_base_position(-0.1_r);

    ElecradCompensator elecrad_comp_{
        .base = [&]{
            switch(motor_role){
                case MotorRole::Pitch:
                    return -0.28_r;
                case MotorRole::Roll:
                    return -0.243_r;
                default:
                    __builtin_unreachable();
            }
        }(),
        .pole_pairs = 7
    };

    HomePositionCompensator home_comp_{
        .base = 0.383_r
    };



    real_t d_volt_ = 0;
    real_t q_volt_ = 0;
    real_t meas_rad_ = 0;

    dsp::Leso leso{dsp::Leso::Config{
        .b0 = 1.3_r,
        .w = 13,
        .fs = FOC_FREQ
    }};

    [[maybe_unused]] auto cb_sensored = [&]{
        ma730.update().examine();
        bmi.update().examine();
        // cb_imu();

        const auto meas_lap = 1-ma730.get_lap_position().examine(); 
        pos_sensor_.update(meas_lap);


        const real_t meas_rad = elecrad_comp_(meas_lap);


        // const auto meas_pos = pos_sensor_.position() - 0.3_r;
        const auto meas_pos = home_comp_({pos_sensor_.position(), meas_lap});
        const auto meas_spd = pos_sensor_.speed();

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
        // scexpr real_t omega = 3 * real_t(TAU);
        // scexpr real_t omega = 1;
        // scexpr real_t amp = 0.1_r;
        // const auto clock_time = clock::time();
        const auto [targ_pos, targ_spd] = ({
            std::make_tuple(
                // amp * sin(omega * clock_time), amp * omega * cos(omega * clock_time)
                base_roll_ * real_t(-1/TAU), base_omega_ * real_t(-1/TAU)
                // 0.3_r * clock_time, 0.3_r
                // 0.0_r, 0_r
            );
        });


        const auto d_volt = 0;
        static constexpr auto MAX_VOLT = 4.7_r;
        const auto q_volt = CLAMP2(
            170.58_r * (targ_pos - meas_pos) +
            25.28_r * (targ_spd - meas_spd)
            // 0
        , MAX_VOLT);
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

        const auto ab_volt = DqVoltage{
            d_volt, 
            CLAMP2(q_volt - leso.get_disturbance(), MAX_VOLT)
            // q_volt
        }.to_ab(meas_rad);
        // ab_volt = DqVoltage{d_volt, q_volt}.to_ab(-0.0_r);
        svpwm.set_ab_volt(
            ab_volt[0], ab_volt[1]
            // amp * cos(omega * clock_time), amp * sin(omega * clock_time)
            // 3.0_r, 0.0_r
        );

        leso.update(meas_spd, q_volt);

        d_volt_ = d_volt;
        q_volt_ = q_volt;
        meas_rad_ = meas_rad;
    };

    adc.attach(hal::AdcIT::JEOC, {0,0}, 
        cb_sensored
    );


    // struct RollGestureDetector{
    //     constexpr operator()(const ){

    //     } 
    // private:
    //     ComplementaryFilter<q16> filter_;
    // };
    auto can_service = []{
        static async::RepeatTimer timer{5ms};
        timer.invoke_if([]{
            const auto msg = hal::CanMsg::from_list(
                hal::CanStdId(0x1234),
                {1,2,3,4}
            );

            can.write(msg);
        });
    };

    auto blink_service = [&]{
        static async::RepeatTimer timer{10ms};
        timer.invoke_if([&]{
            ledr = BoolLevel::from((uint32_t(clock::millis().count()) % 200) > 100);
            ledb = BoolLevel::from((uint32_t(clock::millis().count()) % 400) > 200);
            ledg = BoolLevel::from((uint32_t(clock::millis().count()) % 800) > 400);
        });
    };

    auto repl_service = [&]{
        robots::ReplService repl_service{&DBG_UART, &DBG_UART};

        static const auto list = rpc::make_list(
            "list",
            rpc::make_function("rst", [](){sys::reset();}),
            rpc::make_function("outen", [&](){repl_service.set_outen(true);}),
            rpc::make_function("outdis", [&](){repl_service.set_outen(false);}),
            rpc::make_function("name", [&](){DEBUG_PRINTLN();})
        );

        repl_service.invoke(list);
    };

    auto gesture_service = [&]{
        static constexpr auto DELTA_TIME_MS = 5ms;
        static constexpr auto DELTA_TIME = DELTA_TIME_MS.count() * 0.001_r;
        static constexpr size_t FREQ = 1000ms / DELTA_TIME_MS;
        static async::RepeatTimer timer{DELTA_TIME_MS};
        timer.invoke_if([&]{
                    // DEBUG_PRINTLN_IDLE(curr_sens.raw(), calibrater.result(), calibrater.is_done(), speed_measurer.result());

            // Quat<real_t>::from_gravity(),
            const auto acc = bmi.read_acc().examine();
            const auto gyr = bmi.read_gyr().examine();
            const auto norm_acc = acc.normalized();
            // const auto base_roll = atan2(norm_acc.x, norm_acc.y) - real_t(PI/2);
            const auto base_roll_raw = atan2(norm_acc.x, norm_acc.y) + real_t(PI/2);
            const auto base_omega_raw = gyr.z;

            static dsp::ComplementaryFilter<q20> comp_filter(typename dsp::ComplementaryFilter<q20>::Config{
                .kq = 0.87_r,
                .ko = 0.5_r,
                .fs = FREQ
            });
            const auto base_roll = comp_filter(base_roll_raw, base_omega_raw);
            DEBUG_PRINTLN_IDLE(
                norm_acc.x, norm_acc.y,
                base_roll_raw,
                base_omega_raw,
                base_roll,
                pos_sensor_.position(),
                pos_sensor_.lap_position(),
                pos_sensor_.speed()
                // // leso.get_disturbance(),
                // meas_rad_

            );

            base_roll_ = base_roll;
            base_omega_ = base_omega_raw;
        });
    };

    [[maybe_unused]]
    auto report_service = [&]{ 
        [[maybe_unused]] const auto t = clock::time();
        [[maybe_unused]] const auto uvw_curr = curr_sens.uvw();
        [[maybe_unused]] const auto dq_curr = curr_sens.dq();
        [[maybe_unused]] const auto ab_curr = curr_sens.ab();
        // DEBUG_PRINTLN_IDLE(curr_sens.raw(), calibrater.result(), calibrater.is_done(), speed_measurer.result());

        // DEBUG_PRINTLN_IDLE(
        //     pos_sensor_.position(),
        //     pos_sensor_.lap_position(),
        //     pos_sensor_.speed()
        //     // // leso.get_disturbance(),
        //     // meas_rad_

        // );
        // DEBUG_PRINTLN_IDLE(odo.getPosition(), iq_t<16>(speed_measurer.result()), sin(t), t);
        // if(false)

    };

    while(true){
        repl_service();
        can_service();
        blink_service();
        report_service();
        gesture_service();

    }
}