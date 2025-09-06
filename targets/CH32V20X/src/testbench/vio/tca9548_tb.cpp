#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/VirtualBus/TCA9548A/TCA9548A.hpp"
#include "drivers/IMU/Axis6/MPU6050/MPU6050.hpp"


using namespace ymd;

#define UART hal::uart2

void tca9548_main()
{

    UART.init({576000});
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);
    
    auto i2c = hal::I2cSw(&hal::PA<12>(), &hal::PA<15>());
    i2c.init(400_KHz);

    auto tca = drivers::TCA9548A(&i2c, hal::I2cSlaveAddr<7>::from_u7(0x70));

    auto & vi2c = tca[0];   

    auto & act_i2c = vi2c;

    auto mpu = drivers::MPU6050{&act_i2c};
    
    mpu.init({}).examine();


    while(true){
        mpu.update().examine();
        DEBUG_PRINTLN(clock::millis(), mpu.read_acc().unwrap());
        DEBUG_PRINTLN(clock::millis(), tca.validate());
        clock::delay(20ms);
    }

}