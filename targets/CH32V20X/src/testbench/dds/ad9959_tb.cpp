#include "src/testbench/tb.h"

#include "drivers/DDS/AD9959/ad9959.hpp"

#include "hal/bus/spi/spisw.hpp"
#include "hal/bus/spi/spidrv.hpp"

using namespace ymd;
using namespace ymd::hal;

#if 0
void ad9959_main(){
    auto & cs_gpio = hal::PA<0>();

    SpiSw spisw{
        &SPI1_SCLK_GPIO, 
        &SPI1_MOSI_GPIO, 
        &SPI1_MISO_GPIO, 
        &SPI1_CS_GPIO
    };
    
    auto & spi = spisw;
    spi.init({100000});//maxium baud
    spi.set_bitorder(LSB);

    [[maybe_unused]]
    const auto spi_fd = spi.allocate_cs_gpio(&cs_gpio);

    // timer1.init(1);
    // SpiDrv ad9959_drv{spi, 0};

    // AD9959 ad9959{ad9959_drv};
    // ad9959.init();
}

#endif