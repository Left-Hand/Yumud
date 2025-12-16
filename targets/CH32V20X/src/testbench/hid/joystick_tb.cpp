#include "src/testbench/tb.h"

#include "hal/bus/spi/spisw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/HID/ps2_joystick/ps2_joystick.hpp"

#ifdef SPI1_PRESENT

using namespace ymd;
using namespace ymd::drivers;

#define DBG_UART hal::usart2

void joystick_main(){

    DBG_UART.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000
    });


    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);

    auto SCLK_PIN = SPI1_RM0_SCLK_PIN;
    auto MOSI_PIN = SPI1_RM0_MOSI_PIN;
    auto MISO_PIN = SPI1_RM0_MISO_PIN;
    auto CS_PIN = SPI1_RM0_CS_PIN;

    hal::SpiSw spisw{
        &SCLK_PIN, 
        &MOSI_PIN, 
        &MISO_PIN, 
        &CS_PIN
    };

    auto & spi = spisw;

    spi.init({
        .remap = hal::SPI1_REMAP_PA5_PA6_PA7_PA4,
        .baudrate = hal::NearestFreq(100000)
    });//maxium baud
    if(const auto res = spi.set_bitorder(LSB);
        res.is_err()) PANIC(res.unwrap_err());

    auto spi_cs_pin_ = hal::PA<15>();
    hal::SpiDrv ps2_drv{
        &spi, 
        spi.allocate_cs_pin(&spi_cs_pin_).unwrap()
    };

    Ps2Joystick joystick{ps2_drv};
    joystick.init().examine();

    while(true){
        joystick.update().examine();
        const auto payload = joystick.read_info().examine();
        DEBUG_PRINTLN(
            joystick.devid().examine(),
            payload.left_joystick.to_vec2(),
            payload.right_joystick.to_vec2(), 
            payload.modifiers.left_direction(),
            payload.modifiers.to_bitset()
        );
        clock::delay(10ms);
    }
}
#endif