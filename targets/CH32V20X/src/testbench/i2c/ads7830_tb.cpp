#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/Adc/ADS7830/ads7830.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define DBG_UART hal::uart2
#define SCL_GPIO hal::portB[0]
#define SDA_GPIO hal::portB[1]

using drivers::ADS7830;

void ads7830_main(){
    DBG_UART.init({576000});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.no_brackets();
    // DEBUGGER.no_brackets();

    hal::I2cSw i2c = {&SCL_GPIO, &SDA_GPIO};
    i2c.init(1400_KHz);

    // VL6180X vl6180{i2c, I2cSlaveAddr<7>::from_u7(0)};
    ADS7830 ads7830{&i2c, hal::I2cSlaveAddr<7>::from_u7(0b10010110 >> 1)};
    
    ads7830.init().examine();
    while(true){
        std::array<uint8_t, ADS7830::CHANNEL_COUNT> adc_data;
        for(uint8_t i = 0; i < adc_data.size(); ++i){
            adc_data[i] = ads7830
                .read_pos_channel(ADS7830::ChannelIndex::from_index(i).examine())
                .examine();
        }

        DEBUG_PRINTLN(adc_data);
        clock::delay(5ms);
    }

}