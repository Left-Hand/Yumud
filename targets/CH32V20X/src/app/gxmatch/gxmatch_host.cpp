#include "gxmatch.hpp"
#include "host/host.hpp"


#include "drivers/VirtualIO/PCA9685/pca9685.hpp"

using Sys::t;

namespace gxm{
void host_main(){
    uart2.init(576000);
    auto & logger = uart2;

    auto i2c = I2cSw{portD[2], portC[12]};
    i2c.init(0);
    
    PCA9685 pca{i2c};
    pca.init();
    
    pca.setFrequency(50);
    // pca.setPwm(0, 0, 10);
    auto & pwm = pca[0];
    
    while(true){

        auto duty = real_t(0.5) + (sin(t) >> 1);
        // duty *= real_t(0.2);
        // pwm = duty;
        pwm = real_t(0.5);
        logger.println(duty);
        delay(20);
    }
    
    
};


}