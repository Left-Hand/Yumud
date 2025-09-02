#include "src/testbench/tb.h"

#include "core/clock/time.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"
#include "drivers/VirtualIO/PCA9685/pca9685.hpp"

#include "robots/kinematics/RRS3/rrs3_kinematics.hpp"
#include "robots/repl/repl_service.hpp"
#include "types/transforms/euler.hpp"

#include "dsp/filter/homebrew/debounce_filter.hpp"
#include "dsp/motor_ctrl/tracking_differentiator.hpp"
#include "dsp/motor_ctrl/position_filter.hpp"
#include "dsp/motor_ctrl/ctrl_law.hpp"
#include "drivers/Encoder/ABEncoder.hpp"

using namespace ymd;

namespace ymd::robots{

}

struct PwmAndDirPhy final{
    struct Config{
        Some<hal::TimerOC *> pwm;
        Some<hal::GpioIntf *> dir_gpio;
    };

    explicit PwmAndDirPhy(const Config & cfg):
            pwm_(cfg.pwm.deref()),
            dir_gpio_(cfg.dir_gpio.deref())
        {;}

    void set_dutycycle(const q31 duty){
        if(duty > 0){
            dir_gpio_.set();
            pwm_.set_dutycycle(duty);
        }else{
            dir_gpio_.clr();
            pwm_.set_dutycycle(-duty);
        }
    }
private:
    hal::TimerOC & pwm_;
    hal::GpioIntf & dir_gpio_;
};

struct DualPwmPhy final{
    struct Config{
        Some<hal::TimerOC *> pwm_pos;
        Some<hal::TimerOC *> pwm_neg;
    };

    explicit DualPwmPhy(const Config & cfg):
            pwm_pos_(cfg.pwm_pos.deref()),
            pwm_neg_(cfg.pwm_neg.deref())
        {;}

    void set_dutycycle(const q31 duty){
        if(duty > 0){
            pwm_pos_.set_dutycycle(duty);
            pwm_neg_.set_dutycycle(0);
        }else{
            pwm_pos_.set_dutycycle(0);
            pwm_neg_.set_dutycycle(-duty);
        }
    }
private:
    hal::TimerOC & pwm_pos_;
    hal::TimerOC & pwm_neg_;
};

struct DirAndPulseCounter final{
    constexpr DirAndPulseCounter(){;}
    constexpr void forward_update(const BoolLevel level){
        update(level, false);
    }

    constexpr void backward_update(const BoolLevel level){
        update(level, true);
    }
    constexpr auto count() const {
        return count_;
    }
private:
    int32_t count_ = 0;

    static constexpr dsp::DebounceFilter::Config DEBOUNCE_CONFIG{
        .pipe_length = 8,
        .threshold = 2,
        .polarity = true
    };

    bool last_state_ = false;

    constexpr void update(const BoolLevel level, const bool is_backward){
        const auto state = level.to_bool();
        const bool is_just_upedge = (last_state_ == false) and (state == true);

        if(is_just_upedge){
            if(is_backward) count_--;
            else count_++;
        }
        last_state_ = state;
    }
};

struct PwmAndDirPhy_WithFg final{

    struct Config{
        Some<hal::TimerOC *> pwm;
        Some<hal::GpioIntf *> dir_gpio;
        uint32_t deduction;
        Some<hal::GpioIntf *> fg_gpio;
    };

    explicit PwmAndDirPhy_WithFg(const Config & cfg):
        deducation_(cfg.deduction),
        phy_(PwmAndDirPhy{{cfg.pwm, cfg.dir_gpio}}),
        fg_gpio_(cfg.fg_gpio.deref())
    {}

    void set_dutycycle(const q31 duty){ 
        phy_.set_dutycycle(duty);
        last_duty_ = duty;
    }

    void tick_10khz(){
        if(last_duty_ > 0){
            counter_.forward_update(fg_gpio_.read());
        }else if(last_duty_ < 0) {
            counter_.backward_update(fg_gpio_.read());
        }
    }

    constexpr Angle<q31> get_position() const {
        const auto turns = q16::from_i32((int64_t(counter_.count()) * int64_t(1 << 16)) / deducation_);
        return Angle<q31>::from_turns(turns);
        // return q16(counter_.count()) >> 6;
    }

    constexpr int32_t count() const {
        return counter_.count();
    }
private:
    uint32_t deducation_;
    PwmAndDirPhy phy_;
    hal::GpioIntf & fg_gpio_;
    DirAndPulseCounter counter_;
    real_t last_duty_ = 0;
};

struct DualPwmMotorPhy_WithAbEnc final{

    struct Config{
        Some<hal::TimerOC *> pwm_pos;
        Some<hal::TimerOC *> pwm_neg;
        uint32_t deduction;
        Some<hal::Gpio *> line_a;
        Some<hal::Gpio *> line_b;
    };

    explicit DualPwmMotorPhy_WithAbEnc(const Config & cfg):
        deducation_(cfg.deduction),
        phy_(DualPwmPhy{{cfg.pwm_pos, cfg.pwm_neg}}),
        encoder_(drivers::AbEncoderByGpio{
                drivers::AbEncoderByGpio::Config{cfg.line_a, cfg.line_b}})
    {}

    void set_dutycycle(const q31 duty){ 
        phy_.set_dutycycle(duty);
        last_duty_ = duty;
    }

    void tick_10khz(){
        encoder_.tick();
    }

    constexpr Angle<q31> get_position() const {
        const auto turns = q16::from_i32((int64_t(encoder_.count()) * int64_t(1 << 16)) / deducation_);
        return Angle<q31>::from_turns(turns);
        // return q16(encoder_.count()) >> 6;
    }

    constexpr int32_t count() const {
        return encoder_.count();
    }
private:
    uint32_t deducation_;
    DualPwmPhy phy_;
    drivers::AbEncoderByGpio encoder_;
    real_t last_duty_ = 0;
};

void diffspd_vehicle_main(){
    //PA6 TIM3CH1   左pwm
    //PA7 TIM3CH2   右pwm
    // static constexpr auto UART_BAUD = 115200 * 2;
    static constexpr auto UART_BAUD = 576000;
    static constexpr auto PWM_FREQ = 2 * 1000;
    static constexpr auto TD_CUTOFF_FREQ = 60;
    // static constexpr auto PWM_FREQ = 1000;
    static constexpr auto MOTOR_CTRL_FREQ = PWM_FREQ; 
    // my_can_ring_main();
    auto & DBG_UART = hal::uart2;

    DBG_UART.init({
        .baudrate = UART_BAUD
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets();

    auto & timer = hal::timer3;

    timer.init({
        .freq = PWM_FREQ,
        .mode = hal::TimerCountMode::Up
    });

    timer.enable_arr_sync();


    auto init_pwm = [](hal::TimerOC & pwm){
        pwm.init({.valid_level = LOW});
    };

    auto init_fg_gpio = [](hal::GpioIntf & gpio){
        gpio.inpu();
    };

    auto init_dir_gpio = [](hal::GpioIntf & gpio){
        gpio.outpp(LOW);
    };


    [[maybe_unused]] auto make_gm25_phy = [&]{
        auto & LEFT_PWM = timer.oc<1>();
        auto & RIGHT_PWM = timer.oc<2>();

        auto & LEFT_FG_GPIO = hal::PA<5>();
        auto & RIGHT_FG_GPIO = hal::PB<1>();

        auto & LEFT_DIR_GPIO = hal::PA<1>();
        auto & RIGHT_DIR_GPIO = hal::PB<8>();
        
        init_pwm(LEFT_PWM);
        init_pwm(RIGHT_PWM);

        init_fg_gpio(LEFT_FG_GPIO);
        init_fg_gpio(RIGHT_FG_GPIO);

        init_dir_gpio(LEFT_DIR_GPIO);
        init_dir_gpio(RIGHT_DIR_GPIO);


        return PwmAndDirPhy_WithFg{{
            .pwm = &LEFT_PWM,
            .dir_gpio = &LEFT_DIR_GPIO,
            .deduction = TD_CUTOFF_FREQ,
            .fg_gpio = &LEFT_FG_GPIO
        }};
    };

    [[maybe_unused]] auto make_310_phy = [&]{ 
        auto & POS_PWM = timer.oc<1>();
        auto & NEG_PWM = timer.oc<2>();

        auto & POS_FG_GPIO = hal::PA<0>();
        auto & NEG_FG_GPIO = hal::PA<1>();
        
        init_pwm(POS_PWM);
        init_pwm(NEG_PWM);

        init_fg_gpio(POS_FG_GPIO);
        init_fg_gpio(NEG_FG_GPIO);


        return DualPwmMotorPhy_WithAbEnc{{
            .pwm_pos = &POS_PWM,
            .pwm_neg = &NEG_PWM,
            .deduction = 60,
            .line_a = &POS_FG_GPIO,
            .line_b = &NEG_FG_GPIO
        }};
    };

    // auto motor_phy = make_gm25_phy();
    auto motor_phy = make_310_phy();

    dsp::PositionFilter motor_td_{
        typename dsp::PositionFilter::Config{
            .fs = MOTOR_CTRL_FREQ,
            .r = 45
        }
    };

    auto motor_pulse_detect_cb = [&]{
        motor_phy.tick_10khz();
    };

    dsp::PdCtrlLaw ctrl_law_{
        .kp = 7,
        .kd = 0.3_r
    };

    auto motor_ctrl_cb = [&](){
        motor_td_.update(motor_phy.get_position());

        const auto ctime = clock::time();
        const auto freq = 0.2_r;
        // const auto amp = 0.5_r;
        const auto amp = 1.0_r;
        const auto [targ_position, targ_speed] = std::make_tuple(
            amp * sinpu(ctime * freq) + 9,
            freq * amp * cospu(ctime * freq)
        );

        const auto position = motor_td_.position();
        const auto speed = motor_td_.speed();

        const auto duty = CLAMP2(
            ctrl_law_(targ_position - position, targ_speed - speed),
            0.97_r
        );
        motor_phy.set_dutycycle(duty);
        // motor_phy.set_dutycycle(amp * sinpu(ctime * freq));
    };



    auto report_motor_service = [&]{

        const auto position = motor_td_.position();
        const auto speed = motor_td_.speed();
        DEBUG_PRINTLN_IDLE(
            motor_phy.get_position(),
            motor_phy.count(),
            position, speed
        );
    };

    timer.attach(hal::TimerIT::Update, {0,0}, [&]{
        motor_pulse_detect_cb();
        motor_ctrl_cb();
    });

    while(true){
        report_motor_service();


        // clock::delay(5ms);
    }
}