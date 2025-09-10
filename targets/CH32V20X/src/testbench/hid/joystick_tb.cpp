#include "src/testbench/tb.h"

#include "hal/bus/spi/spisw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/HID/ps2_joystick/ps2_joystick.hpp"

#ifdef ENABLE_SPI1

using namespace ymd;
using namespace ymd::drivers;

#define DBG_UART hal::uart2

void joystick_main(){

    DBG_UART.init({
        .baudrate = 576000
    });


    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);

    auto SCLK_GPIO = SPI1_SCLK_GPIO;
    auto MOSI_GPIO = SPI1_MOSI_GPIO;
    auto MISO_GPIO = SPI1_MISO_GPIO;
    auto CS_GPIO = SPI1_CS_GPIO;

    hal::SpiSw spisw{
        &SCLK_GPIO, 
        &MOSI_GPIO, 
        &MISO_GPIO, 
        &CS_GPIO
    };

    auto & spi = spisw;
    // auto & spi = hal::spi1;

    spi.init({100000});//maxium baud
    if(const auto res = spi.set_bitorder(LSB);  
        res.is_err()) PANIC(res.unwrap_err());

    auto spi_cs_gpio_ = hal::PA<15>();
    hal::SpiDrv ps2_drv{
        &spi, 
        spi.allocate_cs_gpio(&spi_cs_gpio_).unwrap()
    };

    Ps2Joystick joystick{ps2_drv};
    joystick.init().examine();

    while(true){
        joystick.update().examine();
        const auto payload = joystick.read_info().examine();
        DEBUG_PRINTLN(
            joystick.devid().examine(),
            payload.left_joystick.to_position(),
            payload.right_joystick.to_position(), 
            payload.left_direction(),
            payload.modifiers.to_bitset()
        );
        clock::delay(10ms);
    }
}
#endif