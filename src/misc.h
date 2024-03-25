#ifndef __MISC_H__

#define __MISC_H__

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
#include "src/device/Encoder/MagEnc/MA730/ma730.hpp"
#include "dsp/filter/BurrFilter.hpp"
#include "src/exti/exti.hpp"
#include "src/adc/adcs/adc1.hpp"
#include "dsp/filter/LowpassFilter.hpp"
#include "dsp/filter/HighpassFilter.hpp"
#include "dsp/observer/LinearObserver.hpp"
#include "src/device/Encoder/MagEnc/MT6816/mt6816.hpp"
#include "src/device/Modem/Nec/Encoder/nec_encoder.hpp"
#include "device/VirtualIO/AW9523/aw9523.hpp"
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
// #include "SSD1306/ssd1306.hpp"
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
#include "bus/spi/spi1.hpp"
#include "dsp/controller/PID.hpp"
#include "Actuator/Servo/FOC/tinyfoc.hpp"
#include "src/timer/capture_channel.hpp"
#include "dsp/filter/BurrFilter.hpp"
// #include "adc/adcs/adc1.hpp"
#include "device/Actuator/Servo/PwmServo/pwm_servo.hpp"
#include "device/Actuator/Coil/coil.hpp"
#include "types/basis/Basis_t.hpp"
#include "types/quat/Quat_t.hpp"
#include "device/Wireless/Radio/HC12/HC12.hpp"
#include "gpio/port_virtual.hpp"
#include "src/bus/spi/spi_sw.hpp"
#include "VirtualIO/HC595/hc595.hpp"
#include "IMU/Axis6/BMI270/bmi270.hpp"
#include "IMU/Axis6/MPU6050/mpu6050.hpp"
#include "src/nvic/nvic.hpp"
#include "dsp/lut.hpp"
#include "algo/any/any.hpp"
#include "src/device/Encoder/odometer.hpp"
#include "src/crc/crc.hpp"

extern real_t t;

void GPIO_PortC_Init( void );
void GPIO_SW_I2C_Init(void);
void GPIO_SW_I2S_Init(void);
void GLobal_Reset(void);
real_t CalculateFps();
void SysInfo_ShowUp(Printer & uart);
void Systick_Init(void);

void LED_GPIO_Init();

void TIM2_GPIO_Init();
void TIM3_GPIO_Init();
void TIM4_GPIO_Init();
void TIM_Encoder_Init(TIM_TypeDef * TimBase);
void TIM_PWM_Init(TIM_TypeDef * TimBase, const uint16_t arr);
void ADC1_GPIO_Init();
void ADC1_Init();


namespace Sys{
    __fast_inline void Reset(){NVIC_SystemReset();}

    __fast_inline void reCalculateTime(){
        #ifdef USE_IQ
        t.value = millis() * (int)(0.001 * (1 << GLOBAL_Q));
        #else
        t = msTick * (1 / 1000.0f);
        #endif
    }


    __fast_inline real_t getCurrentSeconds(){
        reCalculateTime();
        return t;
    }


    uint64_t getChipId();
    uint32_t getChipIdCrc();
};

#endif