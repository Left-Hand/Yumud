#include "tb.h"

#include "../hal/bus/spi/spisw.hpp"
#include "../drivers/HID/ps2_joystick/ps2_joystick.hpp"


#define JOYSTICK_TB

void joystick_tb(OutputStream & logger){
    #ifdef JOYSTICK_TB

    //  using [[maybe_unused]] Event = Ps2Joystick::JoyStickEvent;

    SpiSw spisw{SPI1_SCLK_GPIO, SPI1_MOSI_GPIO, SPI1_MISO_GPIO, SPI1_CS_GPIO};
    auto & spi = spisw;
    spi.init(100000);//maxium buad
    spi.setBitOrder(LSB);

    SpiDrv ps2_drv{spi, 0};
    Ps2Joystick joystick{ps2_drv};
    joystick.init();

    while(true){
        // SPI1_MISO_GPIO = !SPI1_MISO_GPIO;
        // logger.println("??");
        joystick.update();
        logger.println(joystick.getLeftJoystick(),joystick.getRightJoystick(), joystick.getLeftDirection(), (uint8_t)joystick.id());
        delay(10);
        // delay(100);
            // (uint8_t)joystick.id(), joystick.valueof(Event::LX));
    }
    #endif
}