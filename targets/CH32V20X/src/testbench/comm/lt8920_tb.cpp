#include "src/testbench/tb.h"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/spi/spisw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/Wireless/Radio/LT8920/lt8920.hpp"

#include "core/string/string_view.hpp"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;
void lt8920_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);


    // SpiSw spisw {SPI1_SCLK_GPIO, SPI1_MOSI_GPIO, SPI1_MISO_GPIO};

    auto & spi = spi1;
    // auto & spi = spisw;


    spi.init({2_MHz});
    

    LT8920 lt{&spi, spi.allocate_cs_gpio(&hal::PA<0>()).unwrap()};
    bindSystickCb([&](){
        lt.tick().examine();
    });


    lt.bind_nrst_gpio(hal::PB<0>()).examine();
    lt.init().examine();
    lt.set_data_rate(1_MHz).examine();

    // LT8920::Role role = LT8920::Role::BROADCASTER;
    // lt.setRole(role);
    bool is_rx = (sys::chip::get_chip_id_crc() != 0x5E0799D2);
    
    if(is_rx) lt.start_listen().examine();

    if(is_rx){

    }else{
        auto src = StringView("Hello World!!!");
        while(true){
            // lt.validate();
            // lt.setDataRate(LT8920::DataRate::Kbps125);
            // DEBUG_PRINTLN(lt.isRfSynthLocked());

            lt.write_block(std::span(
                reinterpret_cast<const uint8_t *>(src.data()), 
                src.length())).examine();
            clock::delay(200ms);
            // DEBUG_PRINTLN(src, lt.receivedAck());
        }
    }
}