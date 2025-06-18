#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/Adc/INA3221/ina3221.hpp"


using namespace ymd;
using namespace ymd::hal;

using drivers::INA3221;

#define UART uart2
#define SCL_GPIO hal::portB[0]
#define SDA_GPIO hal::portB[1]

static constexpr double SHUNT_RES = 0.1;
static constexpr double INV_SHUNT_RES = 1 / SHUNT_RES;

void ina3221_main(){
    UART.init({576000});
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets();

    
    auto i2c = hal::I2cSw(SCL_GPIO, SDA_GPIO);
    i2c.init(1200_KHz);

    INA3221 ina = {i2c};

    ina.init().unwrap();

    while(true){
        const auto ch = INA3221::ChannelIndex::CH1;
        ina.update(ch).unwrap();
        ina.update(INA3221::ChannelIndex::CH2).unwrap();
        ina.update(INA3221::ChannelIndex::CH3).unwrap();
        DEBUG_PRINTLN(
            ina.get_bus_volt(ch).unwrap(), 
            ina.get_shunt_volt(ch).unwrap() * real_t(INV_SHUNT_RES)
        );
        clock::delay(1ms);
    }

}