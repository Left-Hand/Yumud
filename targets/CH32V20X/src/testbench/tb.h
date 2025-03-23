#pragma once

#include "core/system.hpp"

namespace ymd{

    class OutputStream;
    class InputStream;
    class IOStream;
}

namespace ymd::hal{
    class Can;
    class UartHw;
    class Spi;
    class GpioIntf;
    class I2c;
    class Uart;
    class DmaChannel;
}

using namespace ymd;
using namespace ymd::hal;


void memory_tb(OutputStream & logger);

void stepper_tb(hal::UartHw & logger);

void lua_tb(OutputStream & logger);
void pwm_tb(OutputStream & logger);

void adc_tb(OutputStream & logger);
void temp_tb(OutputStream & logger);
void temp_tb2(OutputStream & logger);
void filter_tb(OutputStream & logger);

void pmdc_tb();

void lds14_tb(IOStream & logger,Uart & uart);
void gpio_tb(hal::GpioIntf & gpio);
// void pca_tb(IOStream & logger);
void pca_main();
void st77xx_tb(hal::Spi & spi);

void joystick_tb(OutputStream & logger);

void dma_tb(OutputStream & logger, DmaChannel & channel);

void ch9141_tb(IOStream & logger, Uart & uart);

void can_tb(IOStream & logger, hal::Can & can, bool tx_role = true);

void usbcdc_tb();

void at24cxx_tb(OutputStream & logger, hal::I2c & i2c);

void at24cxx_main();

void w25qxx_main();

void eeprom_main();

void flash_main();

void qlz_main();

void uart_main();
void tcs34725_tb(OutputStream & logger, hal::I2c & i2c);

void tcs34725_main();

void dshot_main();

void at24cxx_tb(OutputStream & logger, hal::I2c & i2c);
void i2c_scanner_main();

void dshot_main();

void calc_main();

void float_main();
void oled_main();

void lt8920_main();

void bmi160_main();
void mpu6050_main();

void ma730_main();

void uart_main();

void m3508_main();
void m2006_main();

void math_tb(hal::UartHw & logger);

void zdt_main(hal::UartHw & logger);

void coro_main();
void pmw3901_main();

void can_ring_main();

void paj7620_main();

void proxy_tb();

void curve_tb();


void btree_basic_tb();

void co_ab_main();
void sincos_pwm_main();

void bldc_main();

void rpc_main();

void leetcode_main();

void canopen_main();

void tca9548_main();
void ads1115_main();

void ina3221_main();
void functional_main();

void fmt_main();
void ws2812_main();


void enum_main();

void hive_main();

void rmst_main();

void dsp_main();
void sincos_main();

void lt8960_main();