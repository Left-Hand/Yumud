#include "tb.h"

#include "drivers/Memory/EEprom/AT24CXX/at24c02.hpp"
#include "drivers/Memory/EEprom/AT24CXX/at24c32.hpp"

void at24cxx_tb(OutputStream & logger, I2c & i2c){
    AT24C02 at24(i2c);

    delay(200);
    at24.init();

    if(false){
        at24.store(at24.load(0) + 1, 0);
        delay(20);
        uart2.println(at24.load(0));
    }

    if(false){
        AT24C02_DEBUG("muti store begin");
        constexpr auto begin_addr = 7;
        // constexpr auto end_addr = 15;
        uint8_t data[] = {0, 1, 2, 4};

        at24.store(data, sizeof(data), begin_addr);
        AT24C02_DEBUG("muti store end");
    }

    {
        // constexpr int page_size = 8;
        // Rangei plat = {8, 17};
        // Rangei window = {0, 0};

        AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
        AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
        AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
        AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    }
}

void at24cxx_main(){
    uart1.init(921600);
    auto & logger = uart1;
    logger.setEps(4);
    logger.setRadix(10);
    logger.setSpace(",");

    I2cSw i2csw = I2cSw(portD[1], portD[0]);
    i2csw.init(400000);

    at24cxx_tb(logger, i2csw);

}