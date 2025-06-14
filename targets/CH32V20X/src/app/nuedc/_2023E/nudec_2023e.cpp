#include "src/testbench/tb.h"
#include "dsp/controller/adrc/leso.hpp"

#include "threads.hpp"
#include "tests.hpp"


using namespace ymd;
using namespace ymd::hal;

#define USE_MOCK_SERVO


auto & SERVO_PWMGEN_TIMER = hal::timer3;
auto & DBG_UART = hal::uart2;
static constexpr auto CTRL_FREQ = 50;


namespace nuedc::_2023E{

struct AppConfig{
    ServoConfig yaw_cfg;
    ServoConfig pitch_cfg;
    GimbalPlanner::Config gimbal_cfg;
};

class HwPortBase{
public:
    static auto make_mock_servo(){
        return tests::MockServo();
    }
};

class HwPort:private HwPortBase{
public:
    using Config = AppConfig;

    HwPort(const Config & cfg_):cfg(cfg_){;}

    const Config cfg;

    hal::TimerOC & pwm_yaw = SERVO_PWMGEN_TIMER.oc<1>();
    hal::TimerOC & pwm_pitch = SERVO_PWMGEN_TIMER.oc<2>();

    void setup(){


        SERVO_PWMGEN_TIMER.init({50});

        #ifndef USE_MOCK_SERVO

        pwm_yaw.init({});
        pwm_pitch.init({});

        pwm_yaw.enable_cvr_sync();
        pwm_pitch.enable_cvr_sync();

        pwm_yaw.set_valid_level(HIGH);
        pwm_pitch.set_valid_level(HIGH);
        #endif

    }

    auto make_yaw_servo(){
        #ifndef USE_MOCK_SERVO
        return PwmServo::make_sg90(cfg.yaw_cfg, pwm_yaw);
        #else
        return make_mock_servo();
        #endif
    }

    auto make_pitch_servo(){
        #ifndef USE_MOCK_SERVO
        return PwmServo::make_sg90(cfg.yaw_cfg, pwm_pitch);
        #else
        return make_mock_servo();
        #endif
    }

    auto time(){
        return clock::time();
    }

    void ready(){
        DEBUG_PRINTLN("============");
        DEBUG_PRINTLN("app started");
    }

    template<typename Fn>
    void register_servo_ctl_callback(Fn && callback){

        SERVO_PWMGEN_TIMER.attach(TimerIT::Update, {0, 0}, std::forward<Fn>(callback));
    }
};

struct Dependency{

};
static constexpr auto make_cfg(){
    return AppConfig{
        .yaw_cfg = ServoConfig{
            .min_radian = -0.5_r,
            .max_radian = 0.5_r
        },
        .pitch_cfg = ServoConfig{
            .min_radian = -0.5_r,
            .max_radian = 0.5_r
        },
        .gimbal_cfg = {
            .dyna_cfg = {
                .kp = 2.0_r,
                .kd = 2.0_r,
                .max_spd = 1_r,
                .max_acc = 1_r,
                .fs = CTRL_FREQ
            },
            .kine_cfg = {
                .gimbal_base_height = 0.1_r,
                .gimbal_dist_to_screen = 1.0_r,
                .screen_width = 1.0_r,
                .screen_height = 1.0_r,
            }
        }
    };
}

}


template<size_t Q>
static constexpr iq_t<Q> tpzpu(const iq_t<Q> x){
    return abs(4 * frac(x - iq_t<Q>(0.25)) - 2) - 1;
}


void Kalman_Filter_X(real_t Accel,real_t Gyro)		{
    static constexpr real_t Q_angle=0.001_r;// 过程噪声的协方差
	static constexpr real_t Q_gyro=0.003_r;//0.003 过程噪声的协方差 过程噪声的协方差为一个一行两列矩阵
	static constexpr real_t R_angle=0.5_r;// 测量噪声的协方差 既测量偏差
	static constexpr real_t dt=0.01_r;//                 
	static constexpr real_t  C_0 = 1;
	static real_t Q_bias;
	static real_t PP[2][2] = { { 1, 0 },{ 0, 1 } };
    static real_t Angle_Balance_X;
	Angle_Balance_X+=(Gyro - Q_bias) * dt; //先验估计

	PP[0][0] += Q_angle - PP[0][1] - PP[1][0] * dt;   // Pk-先验估计误差协方差微分的积分
	PP[0][1] += -PP[1][1] * dt;   // =先验估计误差协方差
	PP[1][0] += -PP[1][1] * dt;
	PP[1][1] += Q_gyro * dt;
		
	const real_t Angle_err = Accel - Angle_Balance_X;	//zk-先验估计
	
	const real_t PCt_0 = C_0 * PP[0][0];
	const real_t PCt_1 = C_0 * PP[1][0];
	
	const real_t E = R_angle + C_0 * PCt_0;
	const real_t E_inv = 1 / E;
	const real_t K_0 = PCt_0 * E_inv;
	const real_t K_1 = PCt_1 * E_inv;
	
	const real_t t_0 = PCt_0;
	const real_t t_1 = C_0 * PP[0][1];

	PP[0][0] -= K_0 * t_0;		 //后验估计误差协方差
	PP[0][1] -= K_0 * t_1;
	PP[1][0] -= K_1 * t_0;
	PP[1][1] -= K_1 * t_1;
		
	Angle_Balance_X	+= K_0 * Angle_err;	 //后验估计
	Q_bias	+= K_1 * Angle_err;	 //后验估计
	//angle_dot   = Gyro - Q_bias;	 //输出值(后验估计)的微分=角速度
}

class MockMotor{
public:
    struct Config{
        uint32_t fs;
    };

    MockMotor(const Config & cfg){reconf(cfg);}

    void reconf(const Config & cfg){
        dt_ = 1_r / cfg.fs;
    }

    void update(const real_t u){
        auto & self = *this;
        self.state_ = forward(self, state_, u);
    }

    const auto & get() const {return state_;}
private:
    using Self = MockMotor;
    using State = dsp::StateVector<real_t, 2>;

    State state_;
    real_t dt_;
    static constexpr State forward(const Self & self, const State & x, const real_t u_in){
        // const auto distrb_of_partial = 0;
        // constexpr auto DAMPING = 0;
        // const auto distrb_of_damp = DAMPING * x[1];
        // const auto distrb_of_partial = sinpu(x[0]);
        // const auto u = u_in + distrb_of_partial - distrb_of_damp;
        // const auto u = STEP_TO(u_in + distrb_of_partial - distrb_of_damp, q16(0), 100_q16);
        // const auto u = STEP_TO(u_in + distrb_of_partial - distrb_of_damp, q16(0;
        const auto u = u_in;
        return {
            x[0] + x[1] * self.dt_,
            x[1] + u * self.dt_
        };
    }
};


void nuedc_2023e_main(){
    using namespace nuedc::_2023E;
    DBG_UART.init(576000);
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.no_brackets();
    DEBUGGER.force_sync();
    DEBUGGER.set_eps(4);

    const auto cfg = make_cfg();

    HwPort world{cfg};
    world.setup();

    auto servo_yaw = world.make_yaw_servo();

    auto servo_pitch = world.make_pitch_servo();

    auto gimbal_actuator = GimbalActuatorByLambda({
        .yaw_setter = [&servo_yaw](const MotorCmd cmd){
            servo_yaw.set_motorcmd(cmd);
        },
        .pitch_setter = [&servo_pitch](const MotorCmd cmd){
            servo_pitch.set_motorcmd(cmd);
        }
    });

    // auto gimbal_planner = GimbalPlanner(cfg.gimbal_cfg, gimbal_actuator);

    #if 0

    robots::ReplThread repl_thread = {
        &DBG_UART, &DBG_UART,
        rpc::make_list(
            "list",
            rpc::make_function("rst", [](){sys::reset();}),
            rpc::make_function("outen", [&](){repl_thread.set_outen(true);}),
            rpc::make_function("outdis", [&](){repl_thread.set_outen(false);}),
            rpc::make_function("set_rad", [&](const real_t r1, const real_t r2){
                servo_pitch.set_radian(r1);
                servo_yaw.set_radian(r2);
                DEBUG_PRINTLN(r1, r2);
            }),

            rpc::make_function("get_rad", [&](){
                DEBUG_PRINTLN(
                    servo_pitch.get_radian(),
                    servo_yaw.get_radian()
                );
            })
        )
    };

    #endif

    world.ready();

    // world.register_servo_ctl_callback([&]{
    //     gimbal_actuator.set_gest({0,0});
    // });



    TdVec2 td{{
        .kp = tau * tau,
        .kd = 2 * tau,
        .max_spd = 60.0_r,
        .max_acc = 1000.0_r,
        .fs = 1000
    }};



    [[maybe_unused]]
    auto test_td = [&](const auto t){
        // const auto u = 6 * Vector2q<16>::RIGHT.rotated(real_t(TAU) * t);
        // const auto [x,y] = sincos(real_t(TAU) * t);
        // const auto m = sin(3 * real_t(TAU) * t);
        // const auto [x,y] = sincos(ret);
        // const auto u = Vector2q<16>{CLAMP(70 * x, -30, 30), 6 * y};
        // const auto u = Vector2q<16>{CLAMP(70 * x, -5, 5) + m, 0};
        // const auto u = Vector2q<16>{10 * frac(t * 3), 0};
        const auto u = Vector2q<16>{10 * frac(t), 0};
        
        // const auto u = Vector2q<16>{CLAMP(70 * x, -30, 30), 0};
        // const auto u = Vector2q<16>{6 * x, 0};

        const auto u0 = clock::micros();
        td.update(u);
        const auto u1 = clock::micros();
        DEBUG_PRINTLN(u, td.get()[0][0], td.get()[1][0], td.get()[2], u1 - u0);
    };

    [[maybe_unused]]
    auto test_cs = [&](const auto t){
        const auto tau = 80.0_r;
        // const auto u = 6 * Vector2q<16>::RIGHT.rotated(real_t(TAU) * t);
        // const auto [x,y] = sincos(real_t(TAU) * t);
        // const auto m = sin(3 * real_t(TAU) * t);
        // const auto [x,y] = sincos(ret);
        // const auto u = Vector2q<16>{CLAMP(70 * x, -30, 30), 6 * y};
        // const auto u = Vector2q<16>{CLAMP(70 * x, -5, 5) + m, 0};
        
        // const auto u = 10 * frac(t);
        // const auto u = 10 * sinpu(t);
        // const auto u = 150 * CLAMP2(sin(t/5), 0.7_r);
        // const auto u = 15 * CLAMP2(sin(t), 0.7_r);
        // const auto u = 15 * t + 5 * sin(3 * t);
        // const auto u = 5 * frac(t);
        const auto u = 10 * sign(sin(3 * t));

        // const auto u = Vector2q<16>{CLAMP(70 * x, -30, 30), 0};
        // const auto u = Vector2q<16>{6 * x, 0};

        static dsp::Leso leso{dsp::Leso::Config{
            .b0 = 1,
            .w = 17.8_r,
            .fs = 1000
        }};

        static CommandShaper1 cs{{
            .kp = tau * tau,
            .kd = 2 * tau,
            .max_spd = 40.0_r,
            // .max_acc = 200.0_r,
            // .max_acc = 80.0_r,
            .max_acc = 100.0_r,
            .fs = 1000
        }};


        const auto u0 = clock::micros();
        cs.update(u);
        const auto u1 = clock::micros();

        leso.update(cs.get()[0], u);
        DEBUG_PRINTLN(
            u,
            cs.get()[0],
            leso.get_disturbance(),
            u1 - u0
        //     cs.get()[0], 
        //     cs.get()[1], 
        //     cs.get()[2],
        //     cs.kp_,
        //     cs.max_acc_
        //     // u1 - u0
        );
    };

    static constexpr uint ISR_FREQ = 20000;

    static constexpr auto mc_w = 20.8_q12;
    static CommandShaper1 cs{{
        .kp = mc_w * mc_w,
        .kd = 2 * mc_w,
        .max_spd = 60.0_r,
        // .max_acc = 200.0_r,
        // .max_acc = 120.0_r,
        // .max_acc = 100.0_r,
        // .max_acc = 260.0_r,
        .max_acc = 170.0_r,
        .fs = ISR_FREQ
    }};

    static dsp::Leso leso{dsp::Leso::Config{
        .b0 = 1,
        .w = mc_w / 3,
        .fs = ISR_FREQ
    }};

    static MockMotor motor{{.fs = ISR_FREQ}};
    // uint32_t exe;
    auto & test_gpio = portC[13];
    test_gpio.outpp();
    auto test_leso = [&](const auto t){
        // const auto tau = 80.0_r;

        // const auto p0 = 12 * sign(sin(2 * t));
        // const auto p0 = 0.2_r * int(12 * t);
        // const auto p0 = 2 * frac(3 * t);
        // const auto p0 = 12 * frac(t);
        const auto p0 = 36 * t;
        const auto d = 30 * sign(tpzpu(7 * t));
        // const auto p0 = 12 * tpzpu(t);
        // const auto p0 = CLAMP2(10 * tpzpu(t/4), 5);
        // const auto p0 = 30 * sin(t);

        // const auto u0 = clock::micros();
        test_gpio.clr();

        cs.update(p0);
        const auto p = cs.get()[0];
        const auto v = cs.get()[1];


        static constexpr auto mc_w2 = mc_w;
        static constexpr auto kp = mc_w2 * mc_w2;
        static constexpr auto kd = 2 * mc_w2;
        // const auto u = kd * dsp::adrc::ssqrt(p - motor.get()[0]) + kd * (v - motor.get()[1]);
        // const auto u = kp * (p - motor.get()[0]) + kd * (v - motor.get()[1]);
        const auto u = CLAMP2(kp * (p - motor.get()[0]) + kd * (v - motor.get()[1]), 89);
        // const auto dist_inj = + 0.1_r* sinpu(3 * t);
        // const auto dist_inj = 80 + 30.1_r * sin(10 * t);
        const auto dist_inj = d;
        

        motor.update(u + dist_inj - leso.get_disturbance());
        // motor.update(u + dist_inj - leso.get_disturbance());
        leso.update(motor.get()[1], u);
        // const auto u1 = clock::micros();
        test_gpio.set();

        // DEBUG_PRINTLN(
        //     p0, p, v, u,
        //     motor.get()[0], motor.get()[1],
        //     leso.get_disturbance(),
        //     u1 - u0, dist_inj
        // );

        // exe = u1 - u0;
    };

    clock::delay(20ms);
    // bindSystickCb([&]{
    //     const auto t = time();
    //     // test_td(t);
    //     test_cs(t);
    // });
    
    real_t t = 0.0_r;

    timer1.init({ISR_FREQ});
    timer1.attach(TimerIT::Update, {0,0}, [&]{
        t += (1.0_r / ISR_FREQ);
        test_leso(t);

    });

    while(true){
        clock::delay(1ms);

        DEBUG_PRINTLN(
            //     p0, p, v, u,
                motor.get()[0], motor.get()[1],
                leso.get_disturbance()
            //     u1 - u0, dist_inj
        );
        // const auto t = time();
        // test_td(t);
        // test_cs(t);

        // const real_t t = world.time();
        // repl_thread.process(0);
        // t += 0.001_r;
        // test_td(t);
        // DEBUG_PRINTLN(millis());
        // clock::delay(1ms);
    }
}