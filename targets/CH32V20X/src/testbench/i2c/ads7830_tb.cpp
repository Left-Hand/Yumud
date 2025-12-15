#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"
#include "algebra/vectors/vectorx.hpp"

#include "drivers/Adc/ADS7830/ads7830.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define DBG_UART hal::uart2
#define SCL_PIN hal::PB<0>()
#define SDA_PIN hal::PB<1>()

using drivers::ADS7830;



void ads7830_main(){
    DBG_UART.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000
    });
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.no_brackets(EN);

    auto scl_pin = SCL_PIN;
    auto sda_pin = SDA_PIN;
    hal::I2cSw i2c = hal::I2cSw{&scl_pin, &sda_pin};
    i2c.init({400_KHz});

    ADS7830 ads7830{&i2c, hal::I2cSlaveAddr<7>::from_u7(0b10010110 >> 1)};
    
    using SensorsValue = VectorX<uint8_t, ADS7830::CHANNEL_COUNT>;

    auto get_sensors_value = [&]() -> SensorsValue{
        SensorsValue result = SensorsValue::from_uninitialized();

        auto read_channel = [&](size_t i) -> uint8_t{
            return ads7830.read_pos_channel(
                ADS7830::ChannelSelection::from_nth(i).unwrap()
            ).examine();
        };
    
        for (size_t i = 0; i < 4; i++){
            result[i] = read_channel(i + 4);
        }

        for (size_t i = 4; i < 8; i++){
            result[i] = read_channel(i - 4);
        }

        return result;        
    };


    ads7830.init().examine();
    const auto adc_data_bias = get_sensors_value();
    while(true){
        const auto adc_data = get_sensors_value();

        const auto gray_data = adc_data.substract_bounded(adc_data_bias);
        DEBUG_PRINTLN(gray_data > 15);
        clock::delay(5ms);
    }

}