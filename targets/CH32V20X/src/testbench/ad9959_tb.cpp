#include "tb.h"

#include "../drivers/DDS/AD9959/ad9959.hpp"

#include "../hal/bus/spi/spisw.hpp"
#include "../hal/bus/spi/spidrv.hpp"

void ad9959_main(){
    auto & cs_gpio = portA[0];

    SpiSw spisw{SPI1_SCLK_GPIO, SPI1_MOSI_GPIO, SPI1_MISO_GPIO, SPI1_CS_GPIO};
    auto & spi = spisw;
    spi.init(100000);//maxium buad
    spi.configBitOrder(LSB);
    spi.bindCsPin(cs_gpio, 0);

    timer1.init(1);
    SpiDrv ad9959_drv{spi, 0};

    AD9959 ad9959{ad9959_drv};
    ad9959.init();
}