# Details

Date : 2024-02-07 19:41:55

Directory c:\\RV32\\CH32V203\\LearnADC

Total : 164 files,  20508 codes, 10743 comments, 4953 blanks, all 36204 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [.clang-format](/.clang-format) | YAML | 18 | 9 | 12 | 39 |
| [.eide.usr.ctx.json](/.eide.usr.ctx.json) | JSON | 3 | 0 | 0 | 3 |
| [.eide/debug.files.options.yml](/.eide/debug.files.options.yml) | YAML | 3 | 24 | 5 | 32 |
| [.eide/debug.riscv.gcc.options.json](/.eide/debug.riscv.gcc.options.json) | JSON | 38 | 0 | 0 | 38 |
| [.eide/eide.json](/.eide/eide.json) | JSON | 109 | 0 | 0 | 109 |
| [.eide/env.ini](/.eide/env.ini) | Ini | 14 | 0 | 5 | 19 |
| [LearnADC.code-workspace](/LearnADC.code-workspace) | JSON with Comments | 125 | 0 | 0 | 125 |
| [download.cmd](/download.cmd) | Batch | 3 | 0 | 4 | 7 |
| [dsp/controller/PID.cpp](/dsp/controller/PID.cpp) | C++ | 16 | 0 | 6 | 22 |
| [dsp/controller/PID.hpp](/dsp/controller/PID.hpp) | C++ | 32 | 0 | 12 | 44 |
| [dsp/filter/KalmanFilter.cpp](/dsp/filter/KalmanFilter.cpp) | C++ | 23 | 0 | 2 | 25 |
| [dsp/filter/KalmanFilter.hpp](/dsp/filter/KalmanFilter.hpp) | C++ | 22 | 0 | 12 | 34 |
| [dsp/floatlib/fastsqrt.h](/dsp/floatlib/fastsqrt.h) | C | 58 | 0 | 13 | 71 |
| [dsp/floatlib/floatlib.h](/dsp/floatlib/floatlib.h) | C | 4 | 0 | 3 | 7 |
| [sdk/Core/core_riscv.c](/sdk/Core/core_riscv.c) | C | 125 | 167 | 42 | 334 |
| [sdk/Core/core_riscv.h](/sdk/Core/core_riscv.h) | C | 185 | 145 | 48 | 378 |
| [sdk/Ld/Link.ld](/sdk/Ld/Link.ld) | LinkerScript | 128 | 30 | 30 | 188 |
| [sdk/Peripheral/inc/ch32v20x.h](/sdk/Peripheral/inc/ch32v20x.h) | C | 3,799 | 401 | 618 | 4,818 |
| [sdk/Peripheral/inc/ch32v20x_adc.h](/sdk/Peripheral/inc/ch32v20x_adc.h) | C | 152 | 36 | 31 | 219 |
| [sdk/Peripheral/inc/ch32v20x_bkp.h](/sdk/Peripheral/inc/ch32v20x_bkp.h) | C | 70 | 13 | 9 | 92 |
| [sdk/Peripheral/inc/ch32v20x_can.h](/sdk/Peripheral/inc/ch32v20x_can.h) | C | 194 | 99 | 66 | 359 |
| [sdk/Peripheral/inc/ch32v20x_crc.h](/sdk/Peripheral/inc/ch32v20x_crc.h) | C | 16 | 10 | 6 | 32 |
| [sdk/Peripheral/inc/ch32v20x_dbgmcu.h](/sdk/Peripheral/inc/ch32v20x_dbgmcu.h) | C | 34 | 10 | 7 | 51 |
| [sdk/Peripheral/inc/ch32v20x_dma.h](/sdk/Peripheral/inc/ch32v20x_dma.h) | C | 122 | 33 | 28 | 183 |
| [sdk/Peripheral/inc/ch32v20x_exti.h](/sdk/Peripheral/inc/ch32v20x_exti.h) | C | 60 | 19 | 15 | 94 |
| [sdk/Peripheral/inc/ch32v20x_flash.h](/sdk/Peripheral/inc/ch32v20x_flash.h) | C | 105 | 20 | 18 | 143 |
| [sdk/Peripheral/inc/ch32v20x_gpio.h](/sdk/Peripheral/inc/ch32v20x_gpio.h) | C | 146 | 26 | 18 | 190 |
| [sdk/Peripheral/inc/ch32v20x_i2c.h](/sdk/Peripheral/inc/ch32v20x_i2c.h) | C | 133 | 35 | 32 | 200 |
| [sdk/Peripheral/inc/ch32v20x_iwdg.h](/sdk/Peripheral/inc/ch32v20x_iwdg.h) | C | 27 | 13 | 9 | 49 |
| [sdk/Peripheral/inc/ch32v20x_misc.h](/sdk/Peripheral/inc/ch32v20x_misc.h) | C | 24 | 12 | 8 | 44 |
| [sdk/Peripheral/inc/ch32v20x_opa.h](/sdk/Peripheral/inc/ch32v20x_opa.h) | C | 46 | 15 | 12 | 73 |
| [sdk/Peripheral/inc/ch32v20x_pwr.h](/sdk/Peripheral/inc/ch32v20x_pwr.h) | C | 39 | 14 | 10 | 63 |
| [sdk/Peripheral/inc/ch32v20x_rcc.h](/sdk/Peripheral/inc/ch32v20x_rcc.h) | C | 195 | 29 | 32 | 256 |
| [sdk/Peripheral/inc/ch32v20x_rtc.h](/sdk/Peripheral/inc/ch32v20x_rtc.h) | C | 65 | 12 | 20 | 97 |
| [sdk/Peripheral/inc/ch32v20x_spi.h](/sdk/Peripheral/inc/ch32v20x_spi.h) | C | 128 | 50 | 41 | 219 |
| [sdk/Peripheral/inc/ch32v20x_tim.h](/sdk/Peripheral/inc/ch32v20x_tim.h) | C | 335 | 98 | 74 | 507 |
| [sdk/Peripheral/inc/ch32v20x_usart.h](/sdk/Peripheral/inc/ch32v20x_usart.h) | C | 109 | 45 | 32 | 186 |
| [sdk/Peripheral/inc/ch32v20x_wwdg.h](/sdk/Peripheral/inc/ch32v20x_wwdg.h) | C | 22 | 11 | 7 | 40 |
| [sdk/Peripheral/src/ch32v20x_adc.c](/sdk/Peripheral/src/ch32v20x_adc.c) | C | 531 | 564 | 114 | 1,209 |
| [sdk/Peripheral/src/ch32v20x_bkp.c](/sdk/Peripheral/src/ch32v20x_bkp.c) | C | 102 | 121 | 20 | 243 |
| [sdk/Peripheral/src/ch32v20x_can.c](/sdk/Peripheral/src/ch32v20x_can.c) | C | 712 | 368 | 149 | 1,229 |
| [sdk/Peripheral/src/ch32v20x_crc.c](/sdk/Peripheral/src/ch32v20x_crc.c) | C | 30 | 58 | 10 | 98 |
| [sdk/Peripheral/src/ch32v20x_dbgmcu.c](/sdk/Peripheral/src/ch32v20x_dbgmcu.c) | C | 34 | 54 | 11 | 99 |
| [sdk/Peripheral/src/ch32v20x_dma.c](/sdk/Peripheral/src/ch32v20x_dma.c) | C | 148 | 261 | 22 | 431 |
| [sdk/Peripheral/src/ch32v20x_exti.c](/sdk/Peripheral/src/ch32v20x_exti.c) | C | 87 | 82 | 12 | 181 |
| [sdk/Peripheral/src/ch32v20x_flash.c](/sdk/Peripheral/src/ch32v20x_flash.c) | C | 519 | 340 | 107 | 966 |
| [sdk/Peripheral/src/ch32v20x_gpio.c](/sdk/Peripheral/src/ch32v20x_gpio.c) | C | 356 | 247 | 69 | 672 |
| [sdk/Peripheral/src/ch32v20x_i2c.c](/sdk/Peripheral/src/ch32v20x_i2c.c) | C | 430 | 458 | 85 | 973 |
| [sdk/Peripheral/src/ch32v20x_iwdg.c](/sdk/Peripheral/src/ch32v20x_iwdg.c) | C | 36 | 75 | 10 | 121 |
| [sdk/Peripheral/src/ch32v20x_misc.c](/sdk/Peripheral/src/ch32v20x_misc.c) | C | 63 | 39 | 6 | 108 |
| [sdk/Peripheral/src/ch32v20x_opa.c](/sdk/Peripheral/src/ch32v20x_opa.c) | C | 33 | 46 | 6 | 85 |
| [sdk/Peripheral/src/ch32v20x_pwr.c](/sdk/Peripheral/src/ch32v20x_pwr.c) | C | 181 | 168 | 52 | 401 |
| [sdk/Peripheral/src/ch32v20x_rcc.c](/sdk/Peripheral/src/ch32v20x_rcc.c) | C | 435 | 493 | 91 | 1,019 |
| [sdk/Peripheral/src/ch32v20x_rtc.c](/sdk/Peripheral/src/ch32v20x_rtc.c) | C | 193 | 159 | 22 | 374 |
| [sdk/Peripheral/src/ch32v20x_spi.c](/sdk/Peripheral/src/ch32v20x_spi.c) | C | 289 | 297 | 53 | 639 |
| [sdk/Peripheral/src/ch32v20x_tim.c](/sdk/Peripheral/src/ch32v20x_tim.c) | C | 945 | 1,238 | 169 | 2,352 |
| [sdk/Peripheral/src/ch32v20x_usart.c](/sdk/Peripheral/src/ch32v20x_usart.c) | C | 385 | 354 | 68 | 807 |
| [sdk/Peripheral/src/ch32v20x_wwdg.c](/sdk/Peripheral/src/ch32v20x_wwdg.c) | C | 45 | 81 | 14 | 140 |
| [sdk/Startup/startup_ch32v20x_D6.S](/sdk/Startup/startup_ch32v20x_D6.S) | ARM | 240 | 14 | 12 | 266 |
| [sdk/Startup/startup_ch32v20x_D8.S](/sdk/Startup/startup_ch32v20x_D8.S) | ARM | 256 | 14 | 10 | 280 |
| [sdk/Startup/startup_ch32v20x_D8W.S](/sdk/Startup/startup_ch32v20x_D8W.S) | ARM | 260 | 14 | 12 | 286 |
| [src/AS5600/as5600.hpp](/src/AS5600/as5600.hpp) | C++ | 223 | 0 | 55 | 278 |
| [src/HX711/HX711.c](/src/HX711/HX711.c) | C | 73 | 3 | 16 | 92 |
| [src/HX711/HX711.h](/src/HX711/HX711.h) | C | 28 | 0 | 10 | 38 |
| [src/MA730/ma730.hpp](/src/MA730/ma730.hpp) | C++ | 10 | 0 | 6 | 16 |
| [src/MLX90640/I2C_Driver.c](/src/MLX90640/I2C_Driver.c) | C | 128 | 22 | 31 | 181 |
| [src/MLX90640/I2C_Driver.h](/src/MLX90640/I2C_Driver.h) | C | 17 | 0 | 7 | 24 |
| [src/MLX90640/ML90640_I2C_Driver.c](/src/MLX90640/ML90640_I2C_Driver.c) | C | 42 | 0 | 20 | 62 |
| [src/MLX90640/MLX90640_API.cpp](/src/MLX90640/MLX90640_API.cpp) | C++ | 0 | 1,192 | 264 | 1,456 |
| [src/MLX90640/MLX90640_API.h](/src/MLX90640/MLX90640_API.h) | C | 111 | 16 | 15 | 142 |
| [src/MLX90640/MLX90640_I2C_Driver.h](/src/MLX90640/MLX90640_I2C_Driver.h) | C | 15 | 16 | 6 | 37 |
| [src/MPU6050/mpu6050.cpp](/src/MPU6050/mpu6050.cpp) | C++ | 20 | 8 | 7 | 35 |
| [src/MPU6050/mpu6050.hpp](/src/MPU6050/mpu6050.hpp) | C++ | 42 | 1 | 12 | 55 |
| [src/OV7670/SCCB_Driver.c](/src/OV7670/SCCB_Driver.c) | C | 0 | 131 | 7 | 138 |
| [src/OV7670/SCCB_Driver.h](/src/OV7670/SCCB_Driver.h) | C | 0 | 39 | 18 | 57 |
| [src/PCF8574/pcf8574.hpp](/src/PCF8574/pcf8574.hpp) | C++ | 28 | 0 | 8 | 36 |
| [src/SDcard/SPI2_Driver.c](/src/SDcard/SPI2_Driver.c) | C | 32 | 4 | 10 | 46 |
| [src/SDcard/SPI2_Driver.h](/src/SDcard/SPI2_Driver.h) | C | 14 | 0 | 6 | 20 |
| [src/SGM58031/sgm58031.hpp](/src/SGM58031/sgm58031.hpp) | C++ | 188 | 1 | 37 | 226 |
| [src/SSD1306/ssd1306.cpp](/src/SSD1306/ssd1306.cpp) | C++ | 18 | 0 | 7 | 25 |
| [src/SSD1306/ssd1306.hpp](/src/SSD1306/ssd1306.hpp) | C++ | 92 | 0 | 15 | 107 |
| [src/ST7789V2/st7789.cpp](/src/ST7789V2/st7789.cpp) | C++ | 36 | 3 | 9 | 48 |
| [src/ST7789V2/st7789.hpp](/src/ST7789V2/st7789.hpp) | C++ | 66 | 0 | 19 | 85 |
| [src/ST7789/SPI1_Driver.c](/src/ST7789/SPI1_Driver.c) | C | 127 | 4 | 40 | 171 |
| [src/ST7789/SPI1_Driver.h](/src/ST7789/SPI1_Driver.h) | C | 25 | 0 | 10 | 35 |
| [src/ST7789/font.h](/src/ST7789/font.h) | C | 98 | 1 | 7 | 106 |
| [src/ST7789/st7789.c](/src/ST7789/st7789.c) | C | 419 | 7 | 104 | 530 |
| [src/ST7789/st7789.h](/src/ST7789/st7789.h) | C | 54 | 0 | 21 | 75 |
| [src/TCS34725/tcs34725.hpp](/src/TCS34725/tcs34725.hpp) | C++ | 239 | 1 | 54 | 294 |
| [src/TM8211/tm8211.hpp](/src/TM8211/tm8211.hpp) | C++ | 56 | 2 | 15 | 73 |
| [src/TTP229/TTP229.c](/src/TTP229/TTP229.c) | C | 44 | 2 | 12 | 58 |
| [src/TTP229/TTP229.h](/src/TTP229/TTP229.h) | C | 24 | 0 | 8 | 32 |
| [src/VL53L0X/vl53l0x.cpp](/src/VL53L0X/vl53l0x.cpp) | C++ | 89 | 0 | 16 | 105 |
| [src/VL53L0X/vl53l0x.hpp](/src/VL53L0X/vl53l0x.hpp) | C++ | 44 | 0 | 15 | 59 |
| [src/bus/bus.hpp](/src/bus/bus.hpp) | C++ | 38 | 0 | 12 | 50 |
| [src/bus/bus_inc.h](/src/bus/bus_inc.h) | C | 11 | 0 | 3 | 14 |
| [src/bus/busdrv.hpp](/src/bus/busdrv.hpp) | C++ | 94 | 0 | 17 | 111 |
| [src/bus/i2c/i2c.hpp](/src/bus/i2c/i2c.hpp) | C++ | 8 | 0 | 5 | 13 |
| [src/bus/i2c/i2cSw.hpp](/src/bus/i2c/i2cSw.hpp) | C++ | 165 | 12 | 28 | 205 |
| [src/bus/i2c/i2cdrv.hpp](/src/bus/i2c/i2cdrv.hpp) | C++ | 97 | 1 | 15 | 113 |
| [src/bus/i2s/i2s.hpp](/src/bus/i2s/i2s.hpp) | C++ | 8 | 0 | 5 | 13 |
| [src/bus/i2s/i2sSw.hpp](/src/bus/i2s/i2sSw.hpp) | C++ | 72 | 8 | 26 | 106 |
| [src/bus/i2s/i2sdrv.hpp](/src/bus/i2s/i2sdrv.hpp) | C++ | 14 | 0 | 5 | 19 |
| [src/bus/printer.cpp](/src/bus/printer.cpp) | C++ | 65 | 2 | 8 | 75 |
| [src/bus/printer.hpp](/src/bus/printer.hpp) | C++ | 97 | 1 | 27 | 125 |
| [src/bus/serbus.hpp](/src/bus/serbus.hpp) | C++ | 28 | 13 | 14 | 55 |
| [src/bus/spi/spi.hpp](/src/bus/spi/spi.hpp) | C++ | 9 | 0 | 6 | 15 |
| [src/bus/spi/spi2.cpp](/src/bus/spi/spi2.cpp) | C++ | 71 | 0 | 24 | 95 |
| [src/bus/spi/spi2.hpp](/src/bus/spi/spi2.hpp) | C++ | 62 | 3 | 25 | 90 |
| [src/bus/spi/spi2_hs.cpp](/src/bus/spi/spi2_hs.cpp) | C++ | 5 | 0 | 1 | 6 |
| [src/bus/spi/spi2_hs.hpp](/src/bus/spi/spi2_hs.hpp) | C++ | 20 | 0 | 7 | 27 |
| [src/bus/spi/spidrv.hpp](/src/bus/spi/spidrv.hpp) | C++ | 14 | 0 | 5 | 19 |
| [src/bus/uart/uart1.cpp](/src/bus/uart/uart1.cpp) | C++ | 62 | 0 | 13 | 75 |
| [src/bus/uart/uart1.hpp](/src/bus/uart/uart1.hpp) | C++ | 27 | 0 | 9 | 36 |
| [src/bus/uart/uart2.cpp](/src/bus/uart/uart2.cpp) | C++ | 63 | 0 | 13 | 76 |
| [src/bus/uart/uart2.hpp](/src/bus/uart/uart2.hpp) | C++ | 27 | 0 | 9 | 36 |
| [src/can_test.c](/src/can_test.c) | C | 0 | 231 | 74 | 305 |
| [src/ch32v20x_conf.h](/src/ch32v20x_conf.h) | C | 23 | 9 | 9 | 41 |
| [src/ch32v20x_it.c](/src/ch32v20x_it.c) | C | 12 | 23 | 6 | 41 |
| [src/ch32v20x_it.h](/src/ch32v20x_it.h) | C | 3 | 9 | 6 | 18 |
| [src/clock/clock.c](/src/clock/clock.c) | C | 62 | 19 | 18 | 99 |
| [src/clock/clock.h](/src/clock/clock.h) | C | 58 | 26 | 23 | 107 |
| [src/defines/comm_inc.h](/src/defines/comm_inc.h) | C | 8 | 0 | 4 | 12 |
| [src/defines/math_defs.h](/src/defines/math_defs.h) | C | 66 | 0 | 23 | 89 |
| [src/defines/sys_defs.h](/src/defines/sys_defs.h) | C | 63 | 1 | 19 | 83 |
| [src/defines/user_defs.h](/src/defines/user_defs.h) | C | 12 | 5 | 8 | 25 |
| [src/errors/error_macros.cpp](/src/errors/error_macros.cpp) | C++ | 5 | 0 | 1 | 6 |
| [src/errors/error_macros.h](/src/errors/error_macros.h) | C | 30 | 1 | 7 | 38 |
| [src/gpio/gpio.hpp](/src/gpio/gpio.hpp) | C++ | 94 | 0 | 20 | 114 |
| [src/main.cpp](/src/main.cpp) | C++ | 221 | 36 | 66 | 323 |
| [src/old.c](/src/old.c) | C | 0 | 147 | 34 | 181 |
| [src/system_ch32v20x.c](/src/system_ch32v20x.c) | C | 553 | 283 | 127 | 963 |
| [src/system_ch32v20x.h](/src/system_ch32v20x.h) | C | 12 | 10 | 9 | 31 |
| [tools/wch-interface.cfg](/tools/wch-interface.cfg) | Properties | 1 | 1 | 2 | 4 |
| [tools/wch-target.cfg](/tools/wch-target.cfg) | Properties | 9 | 0 | 5 | 14 |
| [types/basis/Basis.cpp](/types/basis/Basis.cpp) | C++ | 0 | 838 | 137 | 975 |
| [types/basis/Basis.hpp](/types/basis/Basis.hpp) | C++ | 0 | 238 | 63 | 301 |
| [types/buffer/buffer.hpp](/types/buffer/buffer.hpp) | C++ | 26 | 0 | 9 | 35 |
| [types/buffer/pingpongbuf/pingpongbuf_t.hpp](/types/buffer/pingpongbuf/pingpongbuf_t.hpp) | C++ | 45 | 0 | 13 | 58 |
| [types/buffer/ringbuf/ringbuf_t.hpp](/types/buffer/ringbuf/ringbuf_t.hpp) | C++ | 45 | 0 | 13 | 58 |
| [types/buffer/stack/stack_t.hpp](/types/buffer/stack/stack_t.hpp) | C++ | 37 | 0 | 10 | 47 |
| [types/color/color_t.hpp](/types/color/color_t.hpp) | C++ | 182 | 0 | 35 | 217 |
| [types/color/color_t.tpp](/types/color/color_t.tpp) | C++ | 260 | 0 | 46 | 306 |
| [types/complex/complex_t.hpp](/types/complex/complex_t.hpp) | C++ | 102 | 0 | 20 | 122 |
| [types/complex/complex_t.tpp](/types/complex/complex_t.tpp) | C++ | 61 | 0 | 11 | 72 |
| [types/iq/iqt.cpp](/types/iq/iqt.cpp) | C++ | 27 | 2 | 6 | 35 |
| [types/iq/iqt.hpp](/types/iq/iqt.hpp) | C++ | 330 | 0 | 127 | 457 |
| [types/matrix/matrix.cpp](/types/matrix/matrix.cpp) | C++ | 112 | 41 | 19 | 172 |
| [types/matrix/matrix.hpp](/types/matrix/matrix.hpp) | C++ | 27 | 0 | 11 | 38 |
| [types/matrix/matrix.tpp](/types/matrix/matrix.tpp) | C++ | 18 | 0 | 1 | 19 |
| [types/quat/Quat.cpp](/types/quat/Quat.cpp) | C++ | 0 | 177 | 34 | 211 |
| [types/quat/Quat.hpp](/types/quat/Quat.hpp) | C++ | 0 | 113 | 22 | 135 |
| [types/real.cpp](/types/real.cpp) | C++ | 72 | 4 | 14 | 90 |
| [types/real.hpp](/types/real.hpp) | C++ | 31 | 4 | 16 | 51 |
| [types/rect2/rect2_t.hpp](/types/rect2/rect2_t.hpp) | C++ | 26 | 0 | 12 | 38 |
| [types/rect2/rect2_t.tpp](/types/rect2/rect2_t.tpp) | C++ | 34 | 5 | 10 | 49 |
| [types/rgb.h](/types/rgb.h) | C | 117 | 0 | 41 | 158 |
| [types/string/String.cpp](/types/string/String.cpp) | C++ | 574 | 55 | 103 | 732 |
| [types/string/String.hpp](/types/string/String.hpp) | C++ | 157 | 51 | 26 | 234 |
| [types/string/string_utils.c](/types/string/string_utils.c) | C | 142 | 0 | 24 | 166 |
| [types/string/string_utils.h](/types/string/string_utils.h) | C | 21 | 0 | 11 | 32 |
| [types/uint24_t.h](/types/uint24_t.h) | C | 28 | 0 | 13 | 41 |
| [types/vector2/vector2_t.hpp](/types/vector2/vector2_t.hpp) | C++ | 102 | 0 | 28 | 130 |
| [types/vector2/vector2_t.tpp](/types/vector2/vector2_t.tpp) | C++ | 204 | 0 | 42 | 246 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)