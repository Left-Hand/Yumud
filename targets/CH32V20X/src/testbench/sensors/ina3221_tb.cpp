#include "../tb.h"

#include "core/debug/debug.hpp"
#include "core/utils/default.hpp"

#include "hal/conn/i2c/soft/soft_i2c.hpp"
#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/Adc/INA3221/ina3221.hpp"


using namespace ymd;


using drivers::INA3221;

#define UART hal::usart2
#define SCL_PIN hal::PB<0>()
#define SDA_PIN hal::PB<1>()

static constexpr double SHUNT_RES = 0.1;
static constexpr double INV_SHUNT_RES = 1 / SHUNT_RES;

void ina3221_main(){
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);

    auto scl_pin_ = SCL_PIN;
    auto sda_pin_ = SDA_PIN;
    auto i2c = hal::SoftI2c(scl_pin_, sda_pin_);
    i2c.init({hal::NearestFreq(1200_KHz)});

    INA3221 ina{&i2c};

    ina.init(Default).examine();

    while(true){
        const auto ch = INA3221::ChannelSelection::CH1;
        ina.update(ch).examine();
        ina.update(INA3221::ChannelSelection::CH2).examine();
        ina.update(INA3221::ChannelSelection::CH3).examine();
        DEBUG_PRINTLN(
            ina.get_bus_volt_code(ch).examine().to_mv(), 
            ina.get_shunt_volt_code(ch).examine().to_volts() * iq16(INV_SHUNT_RES)
        );
        clock::delay(1ms);
    }

}