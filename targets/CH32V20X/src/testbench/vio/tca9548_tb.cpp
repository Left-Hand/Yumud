#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/VirtualBus/TCA9548A/TCA9548A.hpp"
#include "drivers/IMU/Axis6/MPU6050/MPU6050.hpp"


using namespace ymd;

#define UART hal::usart2
#define SCL_PIN hal::PA<12>()
#define SDA_PIN hal::PA<15>()

void tca9548_main(){

    UART.init({
        .remap = hal::UartRemap::_0,
        .baudrate = hal::NearestFreq(576_KHz), 
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);
    
    auto scl_pin_ = SCL_PIN;
    auto sda_pin_ = SDA_PIN;

    hal::I2cSw i2c{&scl_pin_, &sda_pin_};

    i2c.init({400_KHz});

    auto tca = drivers::TCA9548A(&i2c, hal::I2cSlaveAddr<7>::from_u7(0x70));

    auto & vi2c = tca[0];   

    auto & act_i2c = vi2c;

    using Mems = drivers::MPU6050;
    auto mpu = Mems{&act_i2c};
    
    mpu.init({
        .packge = Mems::Package::MPU6050,
        .acc_fs = Mems::AccFs::_2G,
        .gyr_fs = Mems::GyrFs::_1000deg
    }).examine();


    while(true){
        mpu.update().examine();
        DEBUG_PRINTLN(clock::millis(), mpu.read_acc().unwrap());
        DEBUG_PRINTLN(clock::millis(), tca.validate());
        clock::delay(20ms);
    }

}