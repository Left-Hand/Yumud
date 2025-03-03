#include "../tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/Adc/ADS1115/ads1115.hpp"

#include "drivers/IMU/Axis6/MPU6050/MPU6050.hpp"

using namespace ymd;

void ads1115_main()
{

    uart1.init(576000);
    DEBUGGER.retarget(&uart1);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");
    DEBUGGER.noBrackets();

    
    auto i2c = hal::I2cSw(hal::portA[12], hal::portA[15]);
    i2c.init(400_KHz);

    drivers::ADS1115 ads = {i2c};

    ads.setDataRate(ads.datarate(860).unwarp());
    // ads.setMux(ads.differential(2,3).unwarp());
    ads.setMux(ads.singleend(0).unwarp());
    ads.setPga(drivers::ADS1115::PGA::_1_024V);
    ads.enableContMode();
    ads.startConv();

    while(true){
        // if(ads.ready()){
            DEBUG_PRINTLN(millis(), ads.result().unwarp());
        // }

        // ads.
        delay(2);
    }

}