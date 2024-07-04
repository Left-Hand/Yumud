#include "tb.h"

#include "hal/bus/spi/spisw.hpp"
#include "drivers/HID/ps2_joystick/ps2_joystick.hpp"


#define JOYSTICK_TB

void joystick_tb(OutputStream & logger){
    #ifdef JOYSTICK_TB

    using Event = Ps2Joystick::JoyStickEvent;

    // spi1.init(1000000);
    spi1.bindCsPin(portA[15], 0);

    SpiSw spisw{SPI1_SCLK_Gpio, SPI1_MOSI_Gpio, SPI1_MISO_Gpio, SPI1_CS_Gpio};
    auto & spi = spisw;
    spi.init(100000);
    spi.configBitOrder(false);

    SpiDrv ps2_drv{spi, 0};
    Ps2Joystick joystick{ps2_drv};
    joystick.init();

    while(true){
        // SPI1_MISO_Gpio = !SPI1_MISO_Gpio;
        // logger.println("??");
        joystick.update();
        logger.println(joystick.getLeftJoystick(),joystick.getRightJoystick(), (uint8_t)joystick.id(), joystick.permit);
        delay(10);
        // delay(100);
            // (uint8_t)joystick.id(), joystick.valueof(Event::LX));
    }
    #endif
}