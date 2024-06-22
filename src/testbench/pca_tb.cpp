#include "tb.h"

#include "drivers/VirtualIO/PCA9685/pca9685.hpp"

void pca_tb(IOStream & logger){
    // LDS14 lds14(uart);
    auto & i2c = i2c1;
    // I2cSw i2csw = I2cSw(I2C1_SCL_Gpio, I2C1_SDA_Gpio);
    // i2csw.init(0);
    
    i2c.init(100000);
    PCA9685 pca{I2cDrv{i2c, 0x80}};
    // lds14.run();
    pca.init();
}