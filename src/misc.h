#ifndef __MISC_H__

#define __MISC_H__

#include "defines/defs.h"
#include "bus/bus_inc.h"
#include "bkp/bkp.hpp"
#include "../types/real.hpp"
#include "../types/string/String.hpp"
#include "../types/complex/complex_t.hpp"
#include "../types/vector2/vector2_t.hpp"
#include "../types/color/color_t.hpp"
#include "../types/matrix/matrix.hpp"
#include "MLX90640/MLX90640_API.h"
#include "HX711/HX711.h"
#include "TTP229/TTP229.h"

#include "ST7789/st7789.hpp"
#include "SSD1306/ssd1306.hpp"
#include "MPU6050/mpu6050.hpp"
#include "SGM58031/sgm58031.hpp"
#include "TCS34725/tcs34725.hpp"
#include "VL53L0X/vl53l0x.hpp"
#include "PCF8574/pcf8574.hpp"
#include "AS5600/as5600.hpp"
#include "TM8211/tm8211.hpp"
#include "BH1750/bh1750.hpp"
#include "AT24CXX/at24c32.hpp"
#include "W25QXX/w25qxx.hpp"
#include "LT8920/lt8920.hpp"
#include "MA730/ma730.hpp"
#include "ADXL345/adxl345.hpp"
#include "gpio/gpio.hpp"
#include "memory/flash.hpp"
#include "LT8920/lt8920.hpp"

#include "../types/image/painter.hpp"

extern real_t t;

void GPIO_PortC_Init( void );
void GPIO_SW_I2C_Init(void);
void GPIO_SW_I2S_Init(void);
void GLobal_Reset(void);
real_t CalculateFps();
void SysInfo_ShowUp();
void Systick_Init(void);

__fast_inline void reCalculateTime(){
    #ifdef USE_IQ
    t.value = msTick * (int)(0.001 * (1 << GLOBAL_Q));
    #else
    t = msTick * (1 / 1000.0f);
    #endif
}


#endif