
#include "system.hpp"
using Sys::t;
// I2cSw i2cSw(i2cScl, i2cSda);


// SpiDrv SpiDrvLcd = SpiDrv(spi2_hs, 0);
// SpiDrv spiDrvOled = SpiDrv(spi2, 0);
// SpiDrv spiDrvFlash = SpiDrv(spi1, 0);

// SpiDrv spiDrvRadio = SpiDrv(spi1, 0);
// I2cDrv i2cDrvOled = I2cDrv(i2cSw,(uint8_t)0x78);
// I2cDrv i2cDrvMpu = I2cDrv(i2cSw,(uint8_t)0xD0);
// I2cDrv i2cDrvAdc = I2cDrv(i2c1, 0x90);
// I2cDrv i2cDrvTcs = I2cDrv(i2cSw, 0x52);
// I2cDrv i2cDrvVlx = I2cDrv(i2cSw, 0x52);
// I2cDrv i2cDrvPcf = I2cDrv(i2cSw, 0x4e);
// I2cDrv i2cDrvAS = I2cDrv(i2cSw, 0x6c);

// I2cDrv i2cDrvAS = I2cDrv(i2cSw, 0x36 << 1);
// I2cDrv i2cDrvQm = I2cDrv(i2cSw, 0x1a);
// I2cDrv i2cDrvBm = I2cDrv(i2cSw, 0xec);
// I2cDrv i2cDrvMt = I2cDrv(i2cSw, 0x0C);
// ST7789 tftDisplayer(SpiDrvLcd);
// SSD13XX oledDisPlayer(spiDrvOled);
// MPU6050 mpu(i2cDrvMpu);
// SGM58031 ext_adc(i2cDrvAdc);
// LT8920 radio(spiDrvRadio);
// TCS34725 tcs(i2cDrvTcs);
// VL53L0X vlx(i2cDrvVlx);
// PCF8574 pcf(i2cDrvPcf);
// AS5600 mags(i2cDrvAS);

// W25QXX extern_flash(spiDrvFlash);

// AS5600 mag_sensor(i2cDrvAS);
// QMC5883L earth_sensor(i2cDrvQm);
// BMP280 prs_sensor(i2cDrvBm);
// MT6701 mt_sensor(i2cDrvMt);




// static IOStream & logger = uart2;
//     // flash.load(temp);
//     while(true) __WFI;
//     // while(true){
//         // logger.println("Hi");
//         // delay(100);
//     // }

// }


// class Pmdc{
// protected:
//     PwmChannel & pwm_f;
//     PwmChannel & pwm_b;
// public:
//     Pmdc(PwmChannel & _pwm_f, PwmChannel & _pwm_b):pwm_f(_pwm_f), pwm_b(_pwm_b){;}


// }

#include "drivers/Encoder/ABEncoder.hpp"
#include "drivers/Encoder/OdometerLines.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "dsp/oscillator.hpp"
#include "drivers/Dac/TM8211/tm8211.hpp"

#include <bits/stl_numeric.h>
#include <optional>

#include "hal/bus/bus_inc.h"
#include "robots/buck.hpp"
#include "robots/stepper/stepper.hpp"
#include "defines/defs.h"
#include "bus/bus_inc.h"
#include "bkp/bkp.hpp"
#include "dsp/controller/PID.hpp"
#include "dsp/filter/KalmanFilter.hpp"
#include "../types/real.hpp"
#include "../types/string/String.hpp"
#include "../types/complex/complex_t.hpp"
#include "../types/vector2/vector2_t.hpp"
#include "../types/vector3/vector3_t.hpp"
#include "../types/color/color_t.hpp"
#include "../types/matrix/matrix.hpp"
#include "drivers/Encoder/MagEnc/MA730/ma730.hpp"
#include "dsp/filter/BurrFilter.hpp"
#include "hal/exti/exti.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "dsp/filter/LowpassFilter.hpp"
#include "dsp/filter/HighpassFilter.hpp"
#include "dsp/observer/LinearObserver.hpp"
#include "drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "drivers/Modem/Nec/Encoder/nec_encoder.hpp"
#include "drivers/VirtualIO/AW9523/aw9523.hpp"
// #include "MLX90640/MLX90640_API.h"
// #include "HX711/HX711.h"
// #include "TTP229/TTP229.h"
// #include "AS5600/as5600.hpp"
// #include "MT6701/mt6701.hpp"
// #include "HMC5883L/hmc5883l.hpp"
// #include "QMC5883L/qmc5883l.hpp"
// #include "BMP280/bmp280.hpp"
// #include "HX711/hx711.hpp"
// #include "timer/timer.hpp"
// #include "ST7789/st7789.hpp"
// #include "SSD13XX/ssd1306.hpp"
// #include "MPU6050/mpu6050.hpp"
// #include "SGM58031/sgm58031.hpp"
// #include "TCS34725/tcs34725.hpp"
// #include "VL53L0X/vl53l0x.hpp"
// #include "PCF8574/pcf8574.hpp"
// #include "AS5600/as5600.hpp"
// #include "TM8211/tm8211.hpp"
// #include "BH1750/bh1750.hpp"
// #include "AT24CXX/at24c32.hpp"
// #include "W25QXX/w25qxx.hpp"
// #include "LT8920/lt8920.hpp"
// #include "MA730/ma730.hpp"
// #include "ADXL345/adxl345.hpp"
// #include "gpio/gpio.hpp"
// #include "memory/flash.hpp"
// #include "LT8920/lt8920.hpp"

#include "../types/image/painter.hpp"
#include "stdlib.h"
#include "timer/timers/timer_hw.hpp"
#include "timer/pwm_channel.hpp"
#include "bus/spi/spihw.hpp"
#include "dsp/controller/PID.hpp"
#include "hal/timer/capture_channel.hpp"
#include "dsp/filter/BurrFilter.hpp"
// #include "adc/adcs/adc1.hpp"
#include "drivers/Actuator/Servo/PwmServo/pwm_servo.hpp"
#include "drivers/Actuator/Coil/coil.hpp"
#include "types/basis/Basis_t.hpp"
#include "types/quat/Quat_t.hpp"
#include "drivers/Wireless/Radio/HC12/HC12.hpp"
#include "gpio/port_virtual.hpp"
#include "drivers/VirtualIO/HC595/hc595.hpp"
#include "drivers/IMU/Axis6/BMI270/bmi270.hpp"
#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "hal/nvic/nvic.hpp"
#include "dsp/lut.hpp"
#include "algo/any/any.hpp"
#include "drivers/Encoder/odometer.hpp"
#include "hal/crc/crc.hpp"

#include "apps.h"

using Complex = Complex_t<real_t>;
using Color = Color_t<real_t>;

#include "dsp/pll.hpp"
#include "hal/opa/opa.hpp"
#include "drivers/Adc/INA226/ina226.hpp"

#define I2C_BaudRate 400000


#include "drivers/Encoder/Estimmator.hpp"
#include "drivers/Memory/EEPROM/AT24CXX/at24c02.hpp"

real_t warp_to_halfpi(const real_t & x){
    real_t ret = x;
    while(ret > PI / 2) ret -= PI;
    while(ret < -PI / 2) ret += PI;
    return ret;
}

int main(){

    Sys::Misc::prework();

    // stepper_app();
    // stepper_app_new();
    // StepperTest::stepper_test();

    // image.putseg_h8_unsafe(Vector2i{0,0}, 0x5a, true);
    // image.putseg_v8_unsafe(Vector2i{0,0}, 0x39, true);

    // for(auto & datum : data){
    //     logger.println(datum);
    // }

    Stepper stp;
    stp.init();
    can1.init(Can::BaudRate::Mbps1);
    while(true){
        stp.run();
        can1.write({0x4321, {1,2,3,4}});
    }
    // if(false){
    //     constexpr int page_size = 8;
    //     Rangei plat = {11, 17};
    //     Rangei window = {0, 0};

    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     // AT24C02_DEBUG(plat.gird_part(17, page_size, false));
    // }

    // modem_app();
    // test_app();
    // pmdc_test();
    // SpreadCycle::chopper_test();
    // buck_test();
    // osc_test();
}