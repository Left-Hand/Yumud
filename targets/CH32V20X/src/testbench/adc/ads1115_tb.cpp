#include "../tb.h"

#include "sys/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/Adc/ADS1115/ads1115.hpp"

#include "drivers/IMU/Axis6/MPU6050/MPU6050.hpp"

using namespace ymd;

namespace ymd{
OutputStream & operator << (OutputStream & os, const Result<auto, auto> & result){
    if(result.is_ok()) return os << result.unwrap();
    else return os << result.unwrap_err();
}
}

static void test_result(){
    // Option result = Some(2);
    Result<int, const char *> result = Ok(4);
    // Result<void, int> result = Ok(2);

    auto squared = result 
        | [](auto x) { return (x > 0) ? Result<int, const char *>{Ok{x * x}} : Result<int, const char *>(Err<const char *>("minus")); }  // 返回新的Result
        | [](auto x) { return (x > 9) ? Result<int, const char *>{Ok{x + 1}} : Result<int, const char *>(Err<const char *>("small")); }  // 返回新的Result
    ;
    DEBUG_PRINTLN(squared);

    while(true);
}

void ads1115_main()
{

    uart1.init(576000);
    DEBUGGER.retarget(&uart1);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");
    DEBUGGER.noBrackets();

    test_result();
    auto i2c = hal::I2cSw(hal::portA[12], hal::portA[15]);
    i2c.init(400_KHz);

    drivers::ADS1115 ads = {i2c};

    ads.setDataRate(ads.builder().datarate(860).unwrap());
    ads.setMux(ads.builder().differential(2,3).unwrap());
    // ads.setMux(ads.builder().singleend(0).unwarp());
    ads.setPga(drivers::ADS1115::PGA::_1_024V);
    ads.enableContMode();
    ads.startConv();

    while(true){
        // if(ads.ready()){
            DEBUG_PRINTLN(millis(), ads.result().unwrap());
        // }

        // ads.
        delay(2);
    }

}