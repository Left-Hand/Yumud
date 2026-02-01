#include "src/testbench/tb.h"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "hal/bus/spi/hw_singleton.hpp"
#include "hal/bus/spi/soft/soft_spi.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/Wireless/Radio/LT8920/lt8920.hpp"

#include "core/string/view/string_view.hpp"

using namespace ymd;

using namespace ymd::drivers;
void lt8920_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);


    // SoftSpi SoftSpi {SPI1_SCLK_GPIO, SPI1_MOSI_GPIO, SPI1_MISO_GPIO};

    auto & spi = hal::spi1;
    // auto & spi = SoftSpi;


    spi.init({
        hal::SPI1_REMAP_PA5_PA6_PA7_PA4,
        hal::NearestFreq(2_MHz)
    });
    auto spi_cs_pin_ = hal::PA<0>();

    LT8920 lt{&spi, spi.allocate_cs_pin(&spi_cs_pin_).unwrap()};
    set_systick_handler([&](){
        lt.tick().examine();
    });


    auto nrst_pin_ = hal::PB<0>();
    
    lt.bind_nrst_gpio(nrst_pin_).examine();
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