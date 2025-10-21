#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/Adc/INA3221/ina3221.hpp"


using namespace ymd;


using drivers::INA3221;

#define UART hal::uart2
#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()

static constexpr double SHUNT_RES = 0.1;
static constexpr double INV_SHUNT_RES = 1 / SHUNT_RES;

void ina3221_main(){
    UART.init({576000});
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);

    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;
    auto i2c = hal::I2cSw(&scl_gpio_, &sda_gpio_);
    i2c.init({1200_KHz});

    INA3221 ina{&i2c};

    ina.init(INA3221::Config::from_default()).examine();

    while(true){
        const auto ch = INA3221::ChannelSelection::CH1;
        ina.update(ch).examine();
        ina.update(INA3221::ChannelSelection::CH2).examine();
        ina.update(INA3221::ChannelSelection::CH3).examine();
        DEBUG_PRINTLN(
            ina.get_bus_volt(ch).examine(), 
            ina.get_shunt_volt(ch).examine() * real_t(INV_SHUNT_RES)
        );
        clock::delay(1ms);
    }

}