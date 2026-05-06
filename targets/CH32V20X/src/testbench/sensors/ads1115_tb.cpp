#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/conn/i2c/soft/soft_i2c.hpp"

#include "drivers/Adc/ADS1115/ads1115.hpp"

#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

#define SCL_PIN hal::PA<12>();
#define SDA_PIN hal::PA<15>();



void ads1115_main()
{

    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
    });
    DEBUGGER.retarget(&hal::usart2);
    DEBUGGER.build_config()
        .set_eps(4)
        .set_splitter(",")
        .no_brackets(EN)
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();

    auto scl_pin_ = SCL_PIN;
    auto sda_pin_ = SDA_PIN;
    auto i2c = hal::SoftI2c(scl_pin_, sda_pin_);


    i2c.init({
        .baudrate = hal::NearestFreq(400_KHz)
    });

    drivers::ADS1115 ads{&i2c};

    static constexpr auto builder = drivers::ADS1115::ConfigBuilder();
    static constexpr auto datarate = builder.datarate(860).unwrap();
    static constexpr auto mux = builder.differential(2,3).unwrap();
    ads.set_datarate(datarate).examine();
    ads.set_mux(mux).examine();
    // ads.setMux(ads.builder().singleend(0).unwarp());
    ads.set_pga(drivers::ADS1115::PGA::_1_024V).examine();
    ads.enable_cont_mode(EN).examine();
    ads.start_conv().examine();

    while(true){
        // if(ads.ready()){
            DEBUG_PRINTLN(clock::millis(), ads.get_conversion_result().unwrap());
        // }

        // ads.
        clock::delay(2ms);
    }

}