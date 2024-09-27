#pragma once

#include "sys/core/system.hpp"

#include "hal/bus/spi/spi.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2c.hpp"

using Sys::t;

void memory_tb(OutputStream & logger);

void stepper_tb(UartHw & logger);

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

void w25qxx_main();

void eeprom_main();

void qlz_main();

void uart_main();
void tcs34725_tb(OutputStream & logger, I2c & i2c);

void tcs34725_main();

void dshot_main();

void at24cxx_tb(OutputStream & logger, I2c & i2c);

void dshot_main();

void calc_main();

void float_main();
void oled_main();

void lt8920_main();

void bmi160_main();

void ma730_main();

void uart_main();

void m3508_main();

void math_tb(UartHw & logger);