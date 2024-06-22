#include "tb.h"

#include "drivers/VirtualIO/PCA9685/pca9685.hpp"

void pca_tb(IOStream & logger){
    // LDS14 lds14(uart);
    I2cSw i2csw = I2cSw(portB[7], portB[6]);
    i2csw.init(400000);
    logger.println("hwifhac");
    PCA9685 pca{I2cDrv{i2csw, 0x80}};
    // lds14.run();
    pca.init();
}