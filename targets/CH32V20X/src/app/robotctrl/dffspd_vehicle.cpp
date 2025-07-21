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
#include "types/transforms/euler/euler.hpp"
#include "dsp/filter/homebrew/debounce_filter.hpp"


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

    void set_duty(const q31 duty){
        if(duty > 0){
            dir_gpio_.set();
            pwm_.set_duty(duty);
        }else{
            dir_gpio_.clr();
            pwm_.set_duty(-duty);
        }
    }
private:
    hal::TimerOC & pwm_;
    hal::GpioIntf & dir_gpio_;
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

    dsp::DebounceFilter filter_{DEBOUNCE_CONFIG};
    bool last_state_ = false;

    constexpr void update(const BoolLevel level, const bool is_backward){
        filter_.update(level.to_bool());

        const auto state = filter_.is_high();
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
        uint32_t deduction;
        Some<hal::TimerOC *> pwm;
        Some<hal::GpioIntf *> dir_gpio;
        Some<hal::GpioIntf *> fg_gpio;
    };

    explicit PwmAndDirPhy_WithFg(const Config & cfg):
        deducation_(cfg.deduction),
        phy_(PwmAndDirPhy{{cfg.pwm, cfg.dir_gpio}}),
        fg_gpio_(cfg.fg_gpio.deref())
    {}

    void set_duty(const q31 duty){ 
        phy_.set_duty(duty);
        last_duty_ = duty;
    }

    void tick_10khz(){
        if(last_duty_ == 0) return;
        if(last_duty_ > 0)
            counter_.forward_update(fg_gpio_.read());
        else 
            counter_.backward_update(fg_gpio_.read());
    }

    constexpr q16 get_position() const {
        return q16::from_i32((counter_.count() * (1 << 16)) / deducation_);
    }
private:
    uint32_t deducation_;
    PwmAndDirPhy phy_;
    hal::GpioIntf & fg_gpio_;
    DirAndPulseCounter counter_;
    real_t last_duty_ = 0;
};

void diffspd_vehicle_main(){
    static constexpr auto UART_BAUD = 115200 * 2;
    static constexpr auto PWM_FREQ = 10 * 1000;
    // my_can_ring_main();
    auto & DBG_UART = hal::uart2;


    
    DBG_UART.init({
        .baudrate = UART_BAUD
    });


    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets();

    auto & timer = hal::timer1;
    timer.init({PWM_FREQ});
    timer.enable_arr_sync();

    auto & pwm1 = timer.oc<1>();
    auto & pwm2 = timer.oc<2>();
    
    auto init_pwm = [](hal::TimerOC & pwm){
        pwm.init({.valid_level = LOW});
    };

    init_pwm(pwm1);
    init_pwm(pwm2);

    PwmAndDirPhy_WithFg motor_phy{{
        .pwm = &timer.oc<1>(),
        .dir_gpio = &hal::PA<7>(),
        .fg_gpio = &hal::PA<8>()
    }};

    auto motor_pulse_detect_cb = [&]{
        motor_phy.tick_10khz();
    };

    auto sine_openloop_test_motor_service = [&]{
        const auto ctime = clock::time();
        const auto s = sinpu(ctime);
        motor_phy.set_duty(s);
    };

    timer.attach(hal::TimerIT::Update, {0,0}, motor_pulse_detect_cb);

    while(true){
        sine_openloop_test_motor_service();

        DEBUG_PRINTLN(clock::millis(), motor_phy.get_position());
    }
}