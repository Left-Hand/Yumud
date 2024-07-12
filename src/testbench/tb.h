#pragma once



#include "../sys/core/platform.h"
#include "../hal/flash/flash.hpp"
// #include "../hal/rng/rng.hpp"

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
// 
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

#include "../drivers/Encoder/ABEncoder.hpp"
#include "../drivers/Encoder/OdometerLines.hpp"
#include "../hal/adc/adcs/adc1.hpp"
#include "../dsp/oscillator.hpp"
#include "../drivers/Dac/TM8211/tm8211.hpp"

#include <bits/stl_numeric.h>
#include <optional>

#include "../hal/bus/bus_inc.h"
#include "../robots/buck.hpp"
#include "../robots/stepper/stepper.hpp"

#include "../types/real.hpp"
#include "../types/string/String.hpp"
#include "../types/complex/complex_t.hpp"
#include "../types/vector2/vector2_t.hpp"
#include "../types/vector3/vector3_t.hpp"
#include "../types/color/color_t.hpp"
#include "../types/matrix/matrix.hpp"
#include "../drivers/Encoder/MagEnc/MA730/ma730.hpp"

#include "../hal/exti/exti.hpp"
#include "../hal/adc/adcs/adc1.hpp"

#include "../drivers/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "../drivers/Modem/Nec/Encoder/nec_encoder.hpp"
#include "../drivers/VirtualIO/AW9523/aw9523.hpp"


#include "../types/image/painter.hpp"
#include "stdlib.h"
#include "../hal/timer/capture/capture_channel.hpp"

#include "../drivers/Actuator/Servo/PwmServo/pwm_servo.hpp"
#include "../drivers/Actuator/Coil/coil.hpp"
#include "../types/basis/Basis_t.hpp"
#include "../types/quat/Quat_t.hpp"
#include "../drivers/Wireless/Radio/HC12/HC12.hpp"

#include "../drivers/VirtualIO/HC595/hc595.hpp"
// #include "../drivers/IMU/Axis6/BMI270/bmi270.hpp"
#include "../drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "../hal/nvic/nvic.hpp"
#include "../drivers/Encoder/odometer.hpp"
#include "../hal/crc/crc.hpp"
#include "../hal/opa/opa.hpp"
#include "../drivers/Adc/INA226/ina226.hpp"
#include "../drivers/Encoder/Estimmator.hpp"
#include "../drivers/Memory/EEPROM/AT24CXX/at24cxx.hpp"

void memory_tb(OutputStream & logger);

void stepper_tb(IOStream & logger);

void lua_tb(OutputStream & logger);
void pwm_tb(OutputStream & logger);

void adc_tb(OutputStream & logger);
void temp_tb(OutputStream & logger);
void temp_tb2(OutputStream & logger);
void filter_tb(OutputStream & logger);

void pmdc_tb();

void lds14_tb(IOStream & logger,Uart & uart);
void gpio_tb(GpioConcept & gpio);
void pca_tb(IOStream & logger);
void st77xx_tb(IOStream & logger, Spi & spi);

void joystick_tb(OutputStream & logger);

void dma_tb(OutputStream & logger, DmaChannel & channel);

void ch9141_tb(IOStream & logger, Uart & uart);

void can_tb(IOStream & logger, Can & can, bool tx_role = true);

void usbcdc_tb();

void at24cxx_tb(OutputStream & logger, I2c & i2c);

void at24cxx_main();

void eeprom_main();

void qlz_main();

void uart_main();