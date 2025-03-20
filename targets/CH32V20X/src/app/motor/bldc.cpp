#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/opa/opa.hpp"


#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "drivers/IMU/Axis6/BMI160/bmi160.hpp"
#include "drivers/Encoder/odometer.hpp"
#include "drivers/Actuator/Bridge/MP6540/mp6540.hpp"
#include "drivers/Actuator/SVPWM/svpwm.hpp"
#include "drivers/Actuator/SVPWM/svpwm3.hpp"
#include "drivers/Actuator/Bridge/DRV8301/DRV8301.hpp"

#include "dsp/observer/smo/SmoObserver.hpp"
#include "dsp/observer/lbg/RolbgObserver.hpp"
#include "dsp/observer/nonlinear/NonlinearObserver.hpp"

#include "core/polymorphism/traits.hpp"

#include "utils.hpp"
#include "../digiPW/sogi/spll.hpp"
#include "sogi/sogi.hpp"

#include "ctrl.hpp"

#include "algo/interpolation/cubic.hpp"

#include "rpc.hpp"
#include "src/testbench/tb.h"

#include "robots/rpc/arg_parser.hpp"

#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "CurrentSensor.hpp"
#include "GradeCounter.hpp"

using namespace ymd;
using namespace ymd::drivers;
using namespace ymd::foc;
using namespace ymd::digipw;
using namespace ymd::dsp;
using namespace ymd::intp;



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

class SensorlessEncoder:public EncoderIntf{
protected:
    SensorlessObserverTrait & ob_;
public:
    SensorlessEncoder(
        SensorlessObserverTrait & ob
    ):
        ob_(ob){;}
    real_t getLapPosition() = 0;
    void update() = 0;
    void init() = 0;
    bool stable() = 0;
};

struct TurnSolver{
    uint16_t ta = 0;
    uint16_t tb = 0;
    real_t pa = 0;
    real_t pb = 0;
    real_t va = 0;
    real_t vb = 0;
};

[[maybe_unused]] static real_t demo(uint milliseconds){
    // using Vector2_t<real_t> = CubicInterpolation::Vector2_t<real_t>;
    static TurnSolver turnSolver;
    
    uint32_t turnCnt = milliseconds % 2667;
    uint32_t turns = milliseconds / 2667;
    
    scexpr real_t velPoints[7] = {
        real_t(20)/360, real_t(20)/360, real_t(62.4)/360, real_t(62.4)/360, real_t(20.0)/360, real_t(20.0)/360, real_t(20.0)/360
    };
    
    scexpr real_t posPoints[7] = {
        real_t(1.0f)/360,real_t(106.1f)/360,real_t(108.1f)/360, real_t(126.65f)/360, real_t(233.35f)/360,real_t(359.0f)/360,real_t(361.0f)/360
    };

    scexpr uint tickPoints[7] = {
        0, 300, 400, 500, 2210, 2567, 2667 
    };

    int8_t i = 6;

    while((turnCnt < tickPoints[i]) && (i > -1))
        i--;
    
    turnSolver.ta = tickPoints[i];
    turnSolver.tb = tickPoints[i + 1];
    auto dt = turnSolver.tb - turnSolver.ta;

    turnSolver.va = velPoints[i];
    turnSolver.vb = velPoints[i + 1];
    
    turnSolver.pa = posPoints[i];
    turnSolver.pb = posPoints[i + 1];
    real_t dp = turnSolver.pb - turnSolver.pa;

    real_t _t = ((real_t)(turnCnt  - turnSolver.ta) / dt);
    real_t temp = (real_t)dt / 1000 / dp; 

    real_t yt = 0;

    if((i == 0) || (i == 2) || (i == 4))
        yt = CubicInterpolation::forward(
            Vector2_t<real_t>{real_t(0.4f), real_t(0.4f) * turnSolver.va * temp}, 
            Vector2_t<real_t>(real_t(0.6f), real_t(1.0f) - real_t(0.4f)  * turnSolver.vb * temp), _t);
    else
        yt = _t;

    real_t new_pos =  real_t(turns) + turnSolver.pa + dp * yt;

    return new_pos;
}


class BldcMotor{
public:
    SVPWM3 & svpwm_;
    Odometer & odo_;
    CurrentSensor & curr_sensor_;

    BldcMotor(SVPWM3 & svpwm, Odometer & odo, CurrentSensor & curr_sensor):
        svpwm_(svpwm),
        odo_(odo),
        curr_sensor_(curr_sensor){;}

    using Torque = real_t;
    using Speed = real_t;
    using Position = real_t;

    struct CurrentCtrl{
        DqVoltage update(const DqCurrent targ_curr, const DqCurrent meas_curr){
            return {
                d_pi_ctrl.update(targ_curr.d, meas_curr.d),
                q_pi_ctrl.update(targ_curr.q, meas_curr.q)
            };
        }

        PIController d_pi_ctrl = {
        {
            .kp = 0.0_r,
            .ki = 0.011_r,
            .out_min = -6.0_r,
            .out_max = 6.0_r
        }};
        
        PIController q_pi_ctrl = {
        {
            .kp = 0.0_r,
            .ki = 0.011_r,
            .out_min = -6.0_r,
            .out_max = 6.0_r
        }};
    };

    struct TorqueCtrl{
        DqCurrent update(const Torque targ_torque){
            return {0, targ_torque};
        }
    };

    struct TraditionalSpeedCtrl{
        Torque update(const Speed targ_spd, const Speed meas_spd){
            return {
                speed_pi_ctrl.update(targ_spd, meas_spd)
            };
        }


        PIController speed_pi_ctrl = {{
            .kp = 2.3_r,
            // .kp = 0,
            .ki = 0.0277_r,
            // .ki = 0.0001_r,
            // .out_min = -0.3_r,
            // .out_max = 0.3_r
    
            // .kp = 2.3_r,
            // .ki = 0.009_r,
            .out_min = -0.5_r,
            .out_max = 0.5_r
        }};
    };

    struct TraditionalPositionCtrl{
        Speed update(const Position targ_pos, const Position meas_pos, const Speed meas_spd){
            const auto targ_spd = 0;
            return {
                35.8_r * (targ_pos - meas_pos) + 0.7_r*(targ_spd - meas_spd)
            };
        }


        PIController speed_pi_ctrl = {{
            .kp = 2.3_r,
            // .kp = 0,
            .ki = 0.0277_r,
            // .ki = 0.0001_r,
            // .out_min = -0.3_r,
            // .out_max = 0.3_r
    
            // .kp = 2.3_r,
            // .ki = 0.009_r,
            .out_min = -0.5_r,
            .out_max = 0.5_r
        }};
    };

    CurrentCtrl curr_ctrl_  = {};
    TorqueCtrl torque_ctrl_ = {};
    TraditionalSpeedCtrl spd_ctrl_ = {};
    TraditionalPositionCtrl pos_ctrl_ = {};

    void tick(){
        odo_.update();

        const auto targ_pos = real_t(0);
        const auto lap_pos = odo_.getLapPosition();
        const auto meas_pos = odo_.getPosition();
        const auto meas_spd = odo_.getSpeed();

        const real_t meas_rad = (frac(frac(lap_pos - 0.25_r) * 7) * real_t(TAU));

        curr_sensor_.update(meas_rad);
        const auto meas_dq_curr = curr_sensor_.dq();

        const auto cmd_spd = pos_ctrl_.update(targ_pos, meas_pos, meas_spd);
        const auto cmd_torque = spd_ctrl_.update(cmd_spd, meas_spd);
        const auto cmd_dq_curr = torque_ctrl_.update(cmd_torque);
        const auto cmd_dq_volt = curr_ctrl_.update(cmd_dq_curr, meas_dq_curr);

        const auto cmd_ab_volt = dq_to_ab(cmd_dq_volt, meas_rad);

        svpwm_.setAbVolt(cmd_ab_volt[0], cmd_ab_volt[1]);
    }
private:



};



class CurrentBiasCalibrater{
public:
    struct Config{
        uint period_ticks;
        uint fc;
        uint fs;
    };

    using Lpf = LowpassFilter_t<iq_t<16>>;
    using Lpfs = std::array<Lpf, 3>;

    Lpfs lpfs_ = {};

protected:
    uint period_ticks_;
    uint elapsed_ticks_;
    uint fs_;

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
        // constexpr auto stable_threshold = stable_curr_slewrate / foc_freq;

        // const auto mid_point_diff = ABS(mid_point - last_midp_curr_);
        // last_midp_curr_ = mid_point;

        // curr_stable_checker.update(mid_point_diff < stable_threshold);
    }

    bool done(){
        return elapsed_ticks_ >= period_ticks_;
    }

    UvwCurrent result() const{
        return {
            lpfs_[0].result(),
            lpfs_[1].result(),
            lpfs_[2].result(),
        };
    }
};

class CalibraterOrchestor{

};

__no_inline void init_opa(){
    opa1.init<1,1,1>();
}

// __no_inline void test_gpio(){
//     auto gpio = Gpio::reflect<GpioTags::PortSource::PC, GpioTags::PinSource::_0>().inana();
// }

void bldc_main(){
    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");
    delay(200);
    auto & en_gpio = portA[11];
    auto & slp_gpio = portA[12];

    en_gpio.outpp(LOW);
    slp_gpio.outpp(LOW);

    timer1.init(chopper_freq, TimerMode::CenterAlignedUpTrig);

    auto & pwm_u = timer1.oc(1); 
    auto & pwm_v = timer1.oc(2); 
    auto & pwm_w = timer1.oc(3); 

    timer1.oc(4).init(TimerOcMode::UpValid, false)
                .setOutputState(true)
                .setIdleState(false);
    
    timer1.oc(4).cvr() = timer1.arr() - 1;

    pwm_u.init();
    pwm_v.init();
    pwm_w.init();

    spi1.init(18_MHz);
    spi1.bind_cs_pin(portA[15], 2);
    spi1.bind_cs_pin(portA[0], 0);


    can1.init(1_MHz);

    // BMI160 bmi{spi1, 0};

    // bmi.init();

    MA730 ma730{spi1, 2};
    ma730.init();

    // for(size_t i = 0; i < 1000; ++i) {
    //     bmi.update();
    //     // auto [x,y,z] = bmi.getAcc();
    //     auto [x,y,z] = bmi.getGyr();
    //     DEBUG_PRINTLN(x,y,z);
    //     delay(2);
    //     // DEBUGGER << std::endl;
    // }
    Odometer odo{ma730};
    odo.init();


    MP6540 mp6540{
        {pwm_u, pwm_v, pwm_w},
        {adc1.inj(1), adc1.inj(2), adc1.inj(3)}
    };

    mp6540.init();
    mp6540.setSoRes(1_K);
    
    SVPWM3 svpwm {mp6540};
    

    auto & u_sense = mp6540.ch(1);
    auto & v_sense = mp6540.ch(2);
    auto & w_sense = mp6540.ch(3);
    

    // init_opa();
    init_adc();

    real_t meas_pos = 0;
    real_t meas_spd = 0;
    real_t targ_pos = 0;
    real_t targ_spd = 0;

    // UvwCurrent uvw_curr = {0,0,0};
    real_t est_rad;

    CurrentSensor curr_sens = {u_sense, v_sense, w_sense};
    // CurrentSensor curr_sens = {adc1.inj(1), adc1.inj(2), adc1.inj(3)};
    auto & ledr = portC[13];
    auto & ledb = portC[14];
    auto & ledg = portC[15];
    ledr.outpp(); 
    ledb.outpp(); 
    ledg.outpp();
    portA[7].inana();

    // for(size_t i = 0; i < 400; ++i){
    //     curr_sens.updatUVW();
    //     curr_sens.updateAB();
    //     delay(1);
    // }
    
    // mp6540.setBias(14.68_r,14.68_r,14.62_r);



    en_gpio = true;
    slp_gpio = true;
    // uint32_t dt;

    // std::array<real_t, 2> ab_volt;

    // scexpr real_t r_ohms = 7.1_r;
    // scepxr real_t l_mh = 1.45_r;
    [[maybe_unused]] SmoObserver smo_ob = {0.7_r, 0.04_r, 8.22_r, 0.3_r};
    [[maybe_unused]] RolbgObserver lbg_ob;

    [[maybe_unused]] NonlinearObserver nlr_ob = {
        {
            .phase_inductance = 1.45E-3_r,
            .phase_resistance = 7.1_r,
            .observer_gain = 0.1_r,
            .pm_flux_linkage = 3.58e-4_r,
            .freq = chopper_freq/2,
        }
    };


    real_t hfi_result;
    static int sector_cnt = 0;


    // scexpr iq_t<16> pll_freq = iq_t<16>(0.2);
    LapPosPll pll = {
        {
            // .kp = real_t(2 * pll_freq),
            .kp = 470.17_r,
            // .ki = real_t(pll_freq * pll_freq),
            .ki = 20.3_r,
            .fc = 25000
        }
    };

    odo.inverse();

    real_t mg_meas_rad;
    real_t sl_meas_rad;

    // static PIController pos_pi_ctrl{
    // {
    //     .kp = 27.0_r,
    //     .ki = 0.00_r,
    //     .out_min = -50.6_r,
    //     .out_max = 50.6_r
    // }};

    static PIController speed_pi_ctrl{
    {
        .kp = 2.3_r,
        // .kp = 0,
        .ki = 0.0277_r,
        // .ki = 0.0001_r,
        // .out_min = -0.3_r,
        // .out_max = 0.3_r

        // .kp = 2.3_r,
        // .ki = 0.009_r,
        .out_min = -0.5_r,
        .out_max = 0.5_r
    }};

    //     static PIController speed_pi_ctrl{
    // {
    //     .kp = 2.3_r,
    //     // .kp = 0,
    //     .ki = 0.0277_r,
    //     // .ki = 0.0001_r,
    //     // .out_min = -0.3_r,
    //     // .out_max = 0.3_r

    //     // .kp = 2.3_r,
    //     // .ki = 0.009_r,
    //     .out_min = -0.07_r,
    //     .out_max = 0.07_r
    // }};
    // class Test{
    //     operator void (){

    //     }
    // };



    static PIController d_pi_ctrl{
    {
        .kp = 0.0_r,
        .ki = 0.011_r,
        .out_min = -6.0_r,
        .out_max = 6.0_r
    }};

    static PIController q_pi_ctrl{
    {
        .kp = 0.0_r,
        .ki = 0.011_r,
        .out_min = -6.0_r,
        .out_max = 6.0_r
    }};

    AbVoltage ab_volt;
    uint32_t exe_micros;
    [[maybe_unused]] auto cb = [&]{
        auto m = micros();
        odo.update();

        // targ_spd = 0;
        // targ_pos = 4.25_r * floor(3 * t);
        // targ_pos = 0.2_r * sin(real_t(50 * TAU)*t);
        // auto targ_spd = real_t(10 * TAU) * cos(real_t(50 * TAU) * t);
        // targ_pos = 4 * floor(2*t);
        // targ_pos = 7*t + sin(7*t);
        // targ_pos = demo(2*millis());
        // auto targ_spd = 0.0_r;
        // targ_pos = 10.0_r*floor(2*t);
        // targ_pos = sin(t);
        meas_pos = odo.getPosition();
        pll.update(meas_pos);
        meas_spd = odo.getSpeed();
        // meas_spd = pll.speed();
        // meas_spd = 0;
        const real_t meas_lap = odo.getLapPosition();

        const real_t meas_rad = (frac(frac(meas_lap - 0.25_r) * 7) * real_t(TAU));
        mg_meas_rad = meas_rad;
        curr_sens.update(meas_rad);

        const auto dq_curr = curr_sens.dq();

        #define TEST_MODE_Q_SIN_CURR 1
        #define TEST_MODE_VOLT_POS_CTRL 2
        #define TEST_MODE_WEAK_MAG 3
        #define TEST_MODE_POS_SIN 4
        #define TEST_MODE_SQUARE_SWING 5
        // #define TEST_MODE TEST_MODE_Q_SIN_CURR
        // #define TEST_MODE TEST_MODE_POS_SIN
        #define TEST_MODE TEST_MODE_SQUARE_SWING


        #if (TEST_MODE == TEST_MODE_Q_SIN_CURR)
        const auto d_curr_cmd = 0.0_r;
        const auto d_volt = d_pi_ctrl.update(d_curr_cmd, dq_curr.d);
        const auto q_curr_cmd =  0.2_r * sin(time());
        const auto q_volt = q_pi_ctrl.update(q_curr_cmd, dq_curr.q);
        #elif (TEST_MODE == TEST_MODE_VOLT_POS_CTRL)

        #elif (TEST_MODE == TEST_MODE_WEAK_MAG)
        const auto d_curr_cmd = 0.0_r;
        const auto d_volt = d_pi_ctrl.update(d_curr_cmd, dq_curr.d);
        const auto q_curr_cmd =  0.2_r * sin(time());
        const auto q_volt = q_pi_ctrl.update(q_curr_cmd, dq_curr.q);

        // const auto d_volt = d_pi_ctrl.update(0.2_r, dq_curr.d);
        // const auto q_volt = q_pi_ctrl.update(-0.6_r, dq_curr.q);

        // const auto d_volt = d_pi_ctrl.update(0.0_r, dq_curr.d);
        // const auto d_volt = d_pi_ctrl.update((MAX(ab_volt.length() * 0.03_r - 0.2_r)), dq_curr.d);
        // const auto d_curr_cmd = (meas_spd > 10) ? -CLAMP(ab_volt.length() * 0.03_r - 0.2_r, 0.0_r, 0.7_r) : 0.0_r;
        #elif (TEST_MODE == TEST_MODE_POS_SIN)
        scexpr real_t omega = 3 * real_t(TAU);
        scexpr real_t amp = 1.5_r;
        targ_pos = amp * sin(omega * time());
        targ_spd = amp * omega * cos(omega * time());

        const auto d_curr_cmd = 0.0_r;
        const auto d_volt = d_pi_ctrl.update(d_curr_cmd, dq_curr.d);
        const auto q_curr_cmd = CLAMP(1.258_r * sign_sqrt(targ_pos - meas_pos) + 0.14_r*(targ_spd - meas_spd), -0.7_r, 0.7_r);
        const auto q_volt = q_pi_ctrl.update(q_curr_cmd, dq_curr.q);

        #elif (TEST_MODE == TEST_MODE_SQUARE_SWING)
        scexpr real_t omega = 1 * real_t(TAU);
        scexpr real_t amp = 0.5_r;

        const auto d_curr_cmd = 0.0_r;
        const auto d_volt = d_pi_ctrl.update(d_curr_cmd, dq_curr.d);
        const auto q_curr_cmd = SIGN_AS(amp, sin(omega * time()));
        const auto q_volt = q_pi_ctrl.update(q_curr_cmd, dq_curr.q);
        #endif
        // const auto d_volt = 0;
        // const auto q_volt = q_pi_ctrl.update(0.1_r * sign(sin(t)), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(speed_pi_ctrl.update(int(5 * floor(6 * t)) % 60, meas_spd), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(speed_pi_ctrl.update(20 * sin(30 * t), meas_spd), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(0.1_r, dq_curr.q);
        // targ_pos = 0.002_r*sin(80*t);
        // const auto cmd_spd = 35.8_r * (targ_pos - meas_pos) + 0.7_r*(targ_spd - meas_spd); 
        // const auto cmd_spd = 35.8_r * (targ_pos - meas_pos); 
        // const auto q_volt = q_pi_ctrl.update(speed_pi_ctrl.update(cmd_spd, meas_spd) + 0.06_r * sin(80 * t), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(speed_pi_ctrl.update(cmd_spd, meas_spd), dq_curr.q);

        // struct ShakeParam{
        //     real_t torque;
        //     real_t freq;
        // };

        // const auto base_spd = -7.2_r;
        // // const auto base_torque = -0.02_r;
        // const ShakeParam shake_param = {2.9_r * (meas_pos > 0.2_r ? meas_pos : 0), 50.0_r};
        // const auto addi_torque = shake_param.torque * sin(shake_param.freq * real_t(TAU) * t);
        // const auto q_volt = q_pi_ctrl.update(speed_pi_ctrl.update(base_spd, meas_spd) + addi_torque, dq_curr.q);

        // const auto fra_t = frac(t);
        // const auto q_volt = q_pi_ctrl.update(-0.03_r, dq_curr.q);

        // static int cnt = 0;
        // cnt ++;
        // const auto q_volt = (cnt % 2000 <= 2) ?  6.2_r : 0.0_r;
        // const auto q_volt = q_pi_ctrl.update(base_torque + addi_torque, dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(speed_pi_ctrl.update(targ_spd, meas_spd), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(0.2_r, dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(CLAMP2(0.07_r * (targ_pos - meas_pos),0.3_r), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(0.1_r, dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(0.3_r * sign(2 * frac(t/2) - 1), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(0.1_r, dq_curr.q);

        // const auto d_volt = 0;
        // const auto q_volt = 2;

        ab_volt = dq_to_ab(DqVoltage{d_volt, q_volt}, meas_rad);

        svpwm.setAbVolt(ab_volt[0], ab_volt[1]);

        exe_micros = micros() - m;


        // lbg_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);

        // const auto ab_curr = curr_sens.ab();
        // lbg_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        // pll.update(lbg_ob.theta());
 

        // sl_meas_rad = pll.theta();
        // sl_meas_rad = lbg_ob.theta();
    };

    // struct Measurer{
    //     real_t phase_res;
    //     real_t phase_ind;
    //     size_t measure_times;
    // };

    // Measurer meas;

    // size_t measure_times = 0;
    real_t phase_diff = 0;

    real_t phase_res = 7;
    real_t phase_ind;

    [[maybe_unused]] auto cb_measure = [&]{

        targ_pos = 0;

        curr_sens.update(0);



        scexpr int fs = chopper_freq / 2;
        // scexpr int test_freq = 200;
        scexpr int test_freq = 500;
        scexpr real_t test_volt = 0.2_r;
        // scexpr int test_freq = 1000;
        static int cnt = 0; 
        scexpr int div = fs / test_freq;

        static bool upedge_captured = true;

        cnt++;
        if(cnt >= div){
            cnt = 0;
            upedge_captured = false;
        }
        scexpr real_t omega = real_t((TAU * test_freq) / fs);

        {
            static real_t last_curr = 0;
            real_t this_curr = curr_sens.ab().a;
            // spll.update(this_curr);

            if(upedge_captured == false and last_curr < 0 and this_curr > 0){                
                auto phase_diff_pu = (real_t(cnt) / div);
                if(phase_diff_pu < real_t(0.25)){// 1/4
                    phase_diff = LPFN<8>(phase_diff, phase_diff_pu * real_t(TAU));
                }
                // phase_ind = tan(phase_diff) * phase_res / (real_t(TAU) * test_freq);
                phase_ind = tan(phase_diff) * phase_res * real_t(1/TAU)/ test_freq;


                // phase_ind = spll.theta();
                // phase_ind = real_t(cnt) / div;
                // phase_ind = phase_diff;
                // phase_ind = (real_t(cnt) / div) * real_t(TAU);
                // upedge_captured = true;
            }
            last_curr = this_curr;
        }

        const auto __res = omega * cnt;
        ab_volt = {test_volt * sin(__res),0};
        svpwm.setAbVolt(ab_volt[0], ab_volt[1]);
    };

    [[maybe_unused]] auto cb_sensorless = [&]{

        // targ_pos = real_t(6.0) * sin(2 * t);
        targ_pos = real_t(1.0) * time();

        const auto ab_curr = curr_sens.ab();
        // const auto dq_curr = curr_sens.dq();

        // smo_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        lbg_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        // nlr_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        // pll.update(lbg_ob.theta());
        // sl_meas_rad = pll.theta() + 0.3_r;
        sl_meas_rad = lbg_ob.theta();
        // curr_sens.update(pll.theta());
        curr_sens.update(sl_meas_rad);
        // const auto rad = sl_meas_rad + 3.0_r;

        // const auto d_volt = d_pi_ctrl.update(0.01_r, dq_curr.d);
        // const auto d_volt = d_pi_ctrl.update(0.01_r, dq_curr.d);
        const auto d_volt = 0;
        // const auto d_volt = d_pi_ctrl.update(0.3_r, dq_curr.d);
        // const auto q_volt = q_pi_ctrl.update(0.3_r + 0.24_r * sin(3 * t), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(0.3_r + 0.24_r * sin(3 * t), dq_curr.q);
        // const auto q_volt = q_pi_ctrl.update(-0.3_r + 0.2_r * frac(3 * t), dq_curr.q);
        const auto q_volt = 4;
        // const auto q_volt = q_pi_ctrl.update(-0.3_r, dq_curr.q);

        // ab_volt = {4 * cos(rad), 4 * sin(rad)};
        // ab_volt = dq_to_ab(DqVoltage{d_volt, q_volt}, sl_meas_rad + 1.0_r * sin(t));
        // ab_volt = dq_to_ab(DqVoltage{d_volt, q_volt}, sl_meas_rad + 1.5_r);
        ab_volt = dq_to_ab(DqVoltage{d_volt, q_volt}, sl_meas_rad + 2.0_r);
        svpwm.setAbVolt(ab_volt[0], ab_volt[1]);
    };

    [[maybe_unused]] auto cb_hfi = [&]{

        static int cnt = 0;
        // scexpr int hfi_freq = 4096;
        // scexpr int hfi_freq = 2500;
        // scexpr int hfi_freq = 1024;
        // scexpr int hfi_freq = 512;
        // scexpr int hfi_freq = 256;
        // scexpr int divider = chopper_freq / 2 / hfi_freq;
        scexpr int divider = 8;
        cnt = (cnt + 1) % divider;




        real_t hfi_base_volt = 3.2_r;
        real_t hfi_rad = real_t(TAU) * real_t(cnt) / divider;
        real_t hfi_c = cos(hfi_rad);
        real_t hfi_out = hfi_base_volt * hfi_c;

        real_t openloop_base_volt = 6.0_r;
        real_t openloop_rad = -frac(13.1_r * time())*real_t(TAU);
        real_t openloop_c = cos(openloop_rad);
        real_t openloop_s = sin(openloop_rad);
        // real_t s = sin(hfi_rad);

        ab_volt = {hfi_out + openloop_base_volt * openloop_c, openloop_base_volt * openloop_s};
        svpwm.setAbVolt(ab_volt[0], ab_volt[1]);

        // curr_sens.updatUVW();
        // curr_sens.updateAB();
        // curr_sens.updateDQ(0);
        curr_sens.update(0);
        // real_t mul = curr_sens.ab()[1] * s;
        real_t mul = curr_sens.ab()[1] * hfi_c;
        // real_t last_hfi_result = hfi_result;
        // hfi_result = LPF(last_hfi_result, mul);
        static LowpassFilter_t<real_t> hfi_filter{{.fc = 10, .fs = 10000}};
        hfi_filter.update(mul);
        hfi_result = hfi_filter.result();
        // hfi_result = LPFN<9>(hfi_result, mul);

        // hfi_result = mul;
        
        // dt = micros() - m;
    };


    [[maybe_unused]] auto cb_pulse = [&]{
        static int cnt = 0;

        scexpr real_t pulse_volt = 6;
        scexpr int sustain = (0.0003) * 25000;
        scexpr int dur = (0.02) * 25000;

        cnt ++;
        if(cnt >= sustain + dur){
            cnt = 0;
            sector_cnt = (sector_cnt + 1) % 6;
        }

        const int sector = []() -> int{
            switch(sector_cnt){
                default:
                case 0: return 0;
                case 1: return 3;
                case 2: return 1;
                case 3: return 4;
                case 4: return 2;
                case 5: return 5;
            }
        }();

        real_t pulse_rad = real_t(PI/3) * real_t(sector);

        real_t pulse_s = sin(pulse_rad);
        real_t pulse_c = cos(pulse_rad);

        real_t pulse_out = (cnt < sustain) ? pulse_volt : real_t(0);
        // real_t pulse_out = 0;


        ab_volt = {pulse_out * pulse_c, pulse_out * pulse_s};
        svpwm.setAbVolt(ab_volt[0], ab_volt[1]);

        curr_sens.update(0);
    };

    [[maybe_unused]] auto cb_sing = [&]{
        
        static iq_t<16> sing_t = 0;
        sing_t += iq_t<16>(_iq<16>::from_i32(1));

        real_t sing_rad = 0;

        real_t sing_s = sin(sing_rad);
        real_t sing_c = cos(sing_rad);

        real_t sing_out = 4 * sin(2400 * frac(sing_t) * iq_t<16>(TAU) + 3 * sin(40 * frac(sing_t) * iq_t<16>(TAU)));

        ab_volt = {sing_out * sing_c, sing_out * sing_s};
        svpwm.setAbVolt(ab_volt[0], ab_volt[1]);

        curr_sens.update(0);
    };

    [[maybe_unused]] auto cb_openloop = [&]{
        scexpr auto omega = real_t(22 * TAU);
        const auto amp = real_t(2.2);
        // const auto amp = real_t(2.8) + sin(t);
        // auto theta = omega * t + real_t(12) * sin(2 * real_t(TAU) * t);
        auto theta = omega * time();
        // const auto theta = 0;
        ab_volt = {amp * cos(theta), amp * sin(theta)};
        // ab_volt = {amp, amp};
        svpwm.setAbVolt(ab_volt[0], ab_volt[1]);

        odo.update();
        const real_t meas_lap = odo.getLapPosition();

        const real_t meas_rad = (frac(frac(meas_lap - 0.25_r) * 7) * real_t(TAU));
        mg_meas_rad = meas_rad;

        const auto ab_curr = curr_sens.ab();
        lbg_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        // nlr_ob.update(ab_volt[0], ab_volt[1], ab_curr[0], ab_curr[1]);
        pll.update(lbg_ob.theta());
        // sl_meas_rad = pll.theta() + 0.3_r;
        sl_meas_rad = pll.theta();

        curr_sens.update(meas_rad);

        // sogi.update(curr_sens.ab()[0]);
        // spll.update(curr_sens.ab()[0] * 10);
    };




    bool can_en = false;
    auto list = rpc::make_list(
        "list", 
        // rpc::make_function("pos", [](const real_t duty){DEBUG_PRINTS("duty is set to:", duty)}),
        // rpc::make_function("spd", [](const real_t duty){}),
        rpc::make_function("rst", [](){sys::reset();}),
        rpc::make_property("pos", targ_pos),
        rpc::make_property("spd", targ_spd),
        rpc::make_ro_property("mpos", meas_pos),
        rpc::make_ro_property("mspd", meas_spd),
        rpc::make_property("cen", can_en),

        rpc::make_list(
            "l2", 
            rpc::make_ro_property("mpos", meas_pos),
            rpc::make_ro_property("mspd", meas_spd)
        )
    );

    ArgSplitter splitter;

    DEBUGGER.setSplitter(',');
    DEBUGGER.noBrackets();

    
    // OutputStream::Config

    CurrentBiasCalibrater calibrater = {{
        .period_ticks = 10000,
        .fc = 20,
        .fs = foc_freq
    }};

    // constexpr auto alpha = LowpassFilterD_t<double>::solve_alpha(5.0, foc_freq);
    // LowpassFilterD_t<iq_t<16>> speed_measurer = {
    // LowpassFilterD_t<iq_t<16>> speed_measurer = {
    LowpassFilterD_t<iq_t<14>> speed_measurer = {
        {
            .fc = 10, 
            .fs = foc_freq
        }
    };

    auto measure_bias = [&]{
        
        // mp6540.enable(false);
        curr_sens.capture();
        calibrater.update(curr_sens.raw(), curr_sens.mid());

        odo.update();
        speed_measurer.update(odo.getPosition());

        // mp6540.enable(true);
        if(calibrater.done()) {
            // const auto guard = DEBUGGER.createGuard();
            // DEBUGGER.forceSync();
            // DEBUG_PRINTLN("Done Current Bias Measure!!");
            // DEBUG_PRINTLN(calibrater.result());
            // delay(100);
            // ASSERT(false);
            // delay(1000); sys::reset();
        }
    };

    // adc1.bindCb(AdcIT::JEOC, cb_pulse);
    // adc1.bindCb(AdcIT::JEOC, cb_sing);
    // adc1.bindCb(AdcIT::JEOC, cb_sensorless);
    // adc1.bindCb(AdcIT::JEOC, cb);
    // adc1.bindCb(AdcIT::JEOC, cb_measure);
    // adc1.bindCb(AdcIT::JEOC, cb_openloop);
    // adc1.bindCb(AdcIT::JEOC, cb_hfi);

    adc1.bindCb(AdcIT::JEOC, measure_bias);
    adc1.enableIT(AdcIT::JEOC, {0,0});

    
    while(true){
        // DEBUG_PRINTLN_IDLE(curr_sens.raw(), calibrater.result(), calibrater.done(), speed_measurer.result());
        const auto t = time();
        DEBUG_PRINTLN_IDLE(odo.getPosition(), iq_t<16>(speed_measurer.result()), sin(t), t);
        // if(false)
        {
            auto strs_opt = splitter.update(uart2);
            if(strs_opt.has_value()){
                auto & strs = strs_opt.value();

                DEBUG_PRINTLN("------");
                DEBUG_PRINTS("Inputs:", strs);

                {
                    std::vector<rpc::CallParam> params;
                    params.reserve(strs.size());
                    for(const auto & str  : strs){
                        params.push_back(rpc::CallParam(str));
                    }
                    DEBUGGER.print("->");
                    auto res = list ->call(DEBUGGER, params);
                    DEBUG_PRINTS("\r\n^^Function exited with return code", uint8_t(res));
                    DEBUG_PRINTLN("------");
                }

                splitter.clear();
            }
        }

        ledr = (millis() % 200) > 100;
        ledb = (millis() % 400) > 200;
        ledg = (millis() % 800) > 400;

        // auto _t = real_t(0);

        // DEBUG_PRINTLN(odo.getPosition());
        // DEBUG_PRINTLN((odo.getPosition()), real_t(u_sense), real_t(v_sense), real_t(w_sense));
        // DEBUG_PRINTLN_IDLE((odo.getPosition()), uvw_curr[0],uvw_curr[1], uvw_curr[2], real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // DEBUG_PRINTLN_IDLE((odo.getPosition()), real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // DEBUG_PRINTLN_IDLE((odo.getPosition()), uvw_curr[0],uvw_curr[1], uvw_curr[2]);
        // DEBUG_PRINTLN_IDLE((odo.getPosition()), ab_curr[0],ab_curr[1]);
        // delay(2);
        // DEBUG_PRINTLN(pos, dq_curr[0],dq_curr[1], dt);
        [[maybe_unused]] const auto uvw_curr = curr_sens.uvw();
        [[maybe_unused]] const auto dq_curr = curr_sens.dq();
        [[maybe_unused]] const auto ab_curr = curr_sens.ab();


        // DEBUG_PRINTLN_IDLE(pos, ab_curr[0], ab_curr[1], ab_volt[0], ab_volt[1], smo_ob.getTheta(),  dt > 100 ? 1000 + dt : dt);
        // DEBUG_PRINTLN_IDLE(pos, ab_curr[0], ab_curr[1], lbg_ob._e_alpha, lbg_ob._e_beta,  dt > 100 ? 1000 + dt : dt);
        // DEBUG_PRINTLN_IDLE(ab_curr[0], ab_curr[1], sogi.ab()[0], sogi.ab()[1]);

        // DEBUG_PRINTLN_IDLE(real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // DEBUG_PRINTLN_IDLE(ab_volt[0], ab_volt[1]);
        // DEBUG_PRINTLN_IDLE(ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3, (ADC1->IDATAR1 + ADC1->IDATAR2 + ADC1->IDATAR3)/3);
        // DEBUG_PRINTLN_IDLE(meas_pos, ab_curr[0], ab_curr[1], dq_curr[0], dq_curr[1]);
        // DEBUG_PRINTLN_IDLE(meas_pos, meas_spd, dq_curr[0], dq_curr[1], exe_micros);
        // DEBUG_PRINTLN_IDLE(mg_meas_rad, sl_meas_rad, dq_curr[0], dq_curr[1]);
        // DEBUG_PRINTLN_IDLE(mg_meas_rad, sl_meas_rad, uvw_curr.u, uvw_curr.v, uvw_curr.w);
        // DEBUG_PRINTLN_IDLE(meas_pos, odo.getSpeed(), dq_curr.d, dq_curr.q);
        
        // DEBUG_PRINTLN_IDLE(meas_pos, dq_curr[0], dq_curr[1], d_pi_ctrl.output(), q_pi_ctrl.output(), odo.getSpeed());
        // DEBUG_PRINTLN_IDLE(spll.theta(), ab_curr[0],phase_ind * 1000);
        // DEBUG_PRINTLN_IDLE(meas_pos, mg_meas_rad, sl_meas_rad, ab_curr[0], ab_curr[1], dq_curr[0], dq_curr[1]);
        
        // auto s_curr = [&](){
            //     return uvw_curr.u * uvw_curr.u + uvw_curr.v * uvw_curr.v + uvw_curr.w * uvw_curr.w;
            // }();
            // DEBUG_PRINTLN_IDLE(uvw_curr.u, uvw_curr.v, uvw_curr.w,sector_cnt, s_curr);
            // DEBUG_PRINTLN_IDLE(ab_curr[1], ab_volt[0], hfi_result, acos(hfi_result * real_t(1 / 0.045 )));
            
            // DEBUG_PRINTLN_IDLE(hfi_result);
            // DEBUG_PRINTLN_IDLE(ab_volt[0]);
            // DEBUG_PRINTLN_IDLE(ab_curr[0], ab_curr[1], ab_volt[0]);
            // DEBUG_PRINTLN_IDLE(pos, uvw_curr[0], uvw_curr[1], uvw_curr[2], dq_curr[0], dq_curr[1], targ_pos, pos, smo_ob.getTheta(), dt > 100 ? 1000 + dt : dt);
            // delay(2);
            
            // DEBUG_PRINTLN_IDLE(rad, sin(rad), cos(rad), atan2(sin(rad), cos(rad)));
            // DEBUG_PRINTLN_IDLE(pos, uvw_curr[0], uvw_curr[1], uvw_curr[2], dt > 100 ? 1000 + dt : dt);
            // Sys::Clock::reCalculateTime();
            
        // CanMsg msg = {0x11, std::make_tuple<uint8_t, uint8_t>(0x55, 0xAA)};
        // if(can1.pending() == 0 and can_en) can1.write(msg);

        
        // DEBUG_PRINTLN_IDLE(uvw_curr[0], uvw_curr[1], uvw_curr[2], dq_curr, odo.getPosition()); 
        // DEBUG_PRINTLN_IDLE(1); 
            // DEBUG_PRINTLN_IDLE(ab_curr, dq_curr, can1.available(), can1.getTxErrCnt(), std::setbase(2), 11);
            // DEBUG_PRINTLN_IDLE(ab_curr, dq_curr, can1.available(), can1.getTxErrCnt(), std::setbase(2), CAN1->ERRSR);
            // , real_t(pwm_v), real_t(pwm_w), std::dec, data[0]>>12, data[1] >>12, data[2]>>12);
        // DEBUG_PRINTLN_IDLE(odo.getPosition(), odo.getSpeed(), pll.pos_est_, pll.spd_est_, dq_curr.d, dq_curr.q);
        // delay(2);
        // DEBUGGER.noBrackets(true);
        // DEBUG_PRINTLN_IDLE(odo.getPosition(), Vector2_t<real_t>(1,1));
        // delay(2);

        // DEBUGGER.forceSync();
        // if(false){
        //     const auto guard = DEBUGGER.createGuard();
        //     DEBUGGER.noBrackets(false);
        //     DEBUG_PRINTLN(odo.getPosition(), Vector2_t<real_t>(1,1));
        //     // DEBUGGER.flush();
        // }

        // if(true){
        //     const auto guard = DEBUGGER.createGuard();
        //     // DEBUGGER.setSplitter('|');
        //     DEBUGGER.noBrackets(true);
        //     DEBUG_PRINTLN(odo.getPosition(), Vector2_t<real_t>(1,1));
        //     // DEBUGGER.flush();
        // }
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, adc_data_cache[0], adc_data_cache[1], adc_data_cache[2], (ADC1->IDATAR1 + ADC1->IDATAR2 + ADC1->IDATAR3)/3);
        // (ADC1->IDATAR1 + ADC1->IDATAR2 + ADC1->IDATAR3)/3

        // DEBUG_PRINTLN_IDLE(std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3); 
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3, (ADC1->IDATAR1 + ADC1->IDATAR2 + ADC1->IDATAR3)/3); 
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], ab_curr[0], ab_curr[1], est_rad, rad);
        // DEBUG_PRINTLN(std::setprecision(2), std::dec, int(uvw_curr[0]*100), int(uvw_curr[1]*100), int(uvw_curr[2]*100));
        // DEBUG_PRINTLN(std::setprecision(2), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2]);
        // DEBUG_PRINTLN(std::setprecision(2), std::dec, int(uvw_curr[0]*100));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, ADC1->IDATAR1, ADC1->IDATAR2, ADC1->IDATAR3);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, uvw_curr[0], uvw_curr[1], uvw_curr[2], ab_curr[0], ab_curr[1]);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(adc1.inj(1)), real_t(adc1.inj(2)), real_t(adc1.inj(3)));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(u_sense), real_t(v_sense), real_t(w_sense));
        // pwm_u = sin(t) * 0.5_r + 0.5_r;
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(pwm_u), real_t(pwm_v), real_t(pwm_w));
        // delay(5);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(adc1.inj(1)), uint16_t(adc1.inj(1)));
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, real_t(u_sense), s_lpf_u_curr);
        // auto [a,b] = Vector2_t<real_t>{real_t(0), real_t(0.2)}.rotated(open_rad);
        // DEBUG_PRINTLN(a,b);
        // DEBUG_PRINTLN(std::setprecision(3), std::dec, TIM1->CH1CVR, TIM1->CH4CVR, ADC1->IDATAR1);
        // TIM1->CH4CVR = 1000;
        // cb();
        // delay(10);
        // DEBUG_PRINTLN(spi1.cs_port.isIndexValid(0), spi1.cs_port.isIndexValid(1), spi1.cs_port.isIndexValid(2))
        // DEBUG_PRINTLN("0");
        // bmi.check();
        // delay(20);
    }
}