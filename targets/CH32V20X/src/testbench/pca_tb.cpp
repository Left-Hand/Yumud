#include "tb.h"

#include "../drivers/VirtualIO/PCA9685/pca9685.hpp"

void pca_tb(IOStream & logger){
    // LDS14 lds14(uart);
    // auto & i2c = i2c1;
    I2cSw i2c {portB[6], portB[7]};
    
    i2c.init(100000);
    PCA9685 pca{i2c};
    // lds14.run();
    pca.init();
}