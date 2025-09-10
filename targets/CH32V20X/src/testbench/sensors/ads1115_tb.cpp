#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/Adc/ADS1115/ads1115.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

#define SCL_GPIO hal::PA<12>();
#define SDA_GPIO hal::PA<15>();


void ads1115_main()
{

    hal::uart2.init({576000});
    DEBUGGER.retarget(&hal::uart2);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");

    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;
    auto i2c = hal::I2cSw(&scl_gpio_, &sda_gpio_);
    i2c.init(400_KHz);

    drivers::ADS1115 ads{&i2c};

    ads.set_data_rate(ads.builder().datarate(860).unwrap()).examine();
    ads.set_mux(ads.builder().differential(2,3).unwrap()).examine();
    // ads.setMux(ads.builder().singleend(0).unwarp());
    ads.set_pga(drivers::ADS1115::PGA::_1_024V).examine();
    ads.enable_cont_mode(EN).examine();
    ads.start_conv().examine();

    while(true){
        // if(ads.ready()){
            DEBUG_PRINTLN(clock::millis(), ads.get_voltage().unwrap());
        // }

        // ads.
        clock::delay(2ms);
    }

}