#include "gxmatch.hpp"
#include "host/host.hpp"


#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "hal/timer/instance/timer_hw.hpp"


using Sys::t;

namespace gxm{
void host_main(){
    uart2.init(576000);
    auto & logger = uart2;

    auto i2c = I2cSw{portD[2], portC[12]};
    i2c.init(250_KHz);
    // i2c.init(0);//ok
    
    PCA9685 pca{i2c};
    pca.init();
    
    pca.setFrequency(50, real_t(1.09));
    // pca.setPwm(0, 0, 10);
    auto & pwm = pca[0];

    using MG995 = PwmRadianServo;
    
    MG995 servo{pwm};

    auto config = JointLR::Config{
        .max_rad_delta = real_t(0.01),
        .left_radian_clamp = {0,0},
        .right_radian_clamp = {0,0},
    };
    
    JointLR joint{
        servo, 
        config
    };

    
    auto & timer = timer2;
    timer.init(50);
    timer.bindCb(TimerUtils::IT::Update, [&](){joint.tick();});
    timer.enableIt(TimerUtils::IT::Update, NvicPriority{0,0});
     
    while(true){

        // auto duty = real_t(0.5) + (sin(t) >> 1);
        // duty = LERP(real_t(0.025), real_t(0.125), duty);
        // duty = (real_t(0.025) + real_t(0.125))/2;
        // duty *= real_t(0.2);
        // pwm = duty;
        // pwm2 = duty;

        joint.setRadian(real_t(PI/2) * (1 + sin(t)));
        // logger.println(joint.getRadian());
        // pwm = real_t(0.5);
        // logger.println(duty);
        delay(20);
    }
    
    
};


}