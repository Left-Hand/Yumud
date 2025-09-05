#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/clock/clock.hpp"
#include "core/math/realmath.hpp"
#include "core/stream/ostream.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/timer/pwm/gpio_pwm.hpp"
#include "hal/timer/instance/timer_hw.hpp"



#define PWM_TB_GPIO
// #define PWM_TB_TIMEROC
// #define PWM_TB_CO


using namespace ymd;
using namespace ymd::hal;

void pwm_tb(OutputStream & logger){

    timer1.init({36000});
    #ifdef PWM_TB_GPIO
    GpioPwm pwm{hal::PA<8>()};
    pwm.init(32);

    timer1.enable_it(TimerIT::Update, {0,0});
    timer1.bind_cb(TimerIT::Update, [&](){pwm.tick();});

    #endif

    #ifdef PWM_TB_TIMEROC
    PwmIntf & pwm = timer1.oc(1);
    

    #ifdef PWM_TB_CO
    auto & co = timer1.ocn(1);
    co.init();
    co.setPolarity(HIGH);
    co.setIdleState(HIGH);

    timer1.initBdtr();
    #endif
    #endif

    while(true){
        const auto t = clock::time();
        logger.println(t, pwm.cnt(), pwm.cvr());
        pwm.set_dutycycle(0.5_q * sin(4 * t) + 0.5_q);
        clock::delay(100us);
    }

}