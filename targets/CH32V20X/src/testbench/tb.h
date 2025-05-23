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


void memory_tb(ymd::OutputStream & logger);

void stepper_tb(ymd::hal::UartHw & logger);

void lua_tb(ymd::OutputStream & logger);
void pwm_tb(ymd::OutputStream & logger);

void adc_tb(ymd::OutputStream & logger);
void temp_tb(ymd::OutputStream & logger);
void temp_tb2(ymd::OutputStream & logger);
void filter_tb(ymd::OutputStream & logger);

void pmdc_tb();

void lds14_tb(ymd::IOStream & logger,ymd::hal::Uart & uart);
void gpio_tb(ymd::hal::GpioIntf & gpio);
// void pca_tb(ymd::IOStream & logger);
void pca_main();
void st77xx_tb(ymd::hal::Spi & spi);

void joystick_tb(ymd::OutputStream & logger);

void dma_tb(ymd::OutputStream & logger, ymd::hal::DmaChannel & channel);

void ch9141_tb(ymd::IOStream & logger, ymd::hal::Uart & uart);

void can_tb(ymd::IOStream & logger, ymd::hal::Can & can, bool tx_role = true);

void usbcdc_tb();

void at24cxx_tb(ymd::OutputStream & logger, ymd::hal::I2c & i2c);

void at24cxx_main();

void w25qxx_main();

void eeprom_main();

void flash_main();

void qlz_main();

void uart_main();
void tcs34725_tb(ymd::OutputStream & logger, ymd::hal::I2c & i2c);

void tcs34725_main();

void dshot_main();

void at24cxx_tb(ymd::OutputStream & logger, ymd::hal::I2c & i2c);
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

void math_tb(ymd::hal::UartHw & logger);

void zdt_main(ymd::hal::UartHw & logger);

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

void light_tracking_main();
void cubic_main();
void gxm_new_energy_main();
void rrs3_robot_main();

void nuedc_2023e_main();
void tm1637_main();
void vl6180x_main();
void svpwm3_main();
void phase3pwm_main();
void pfm_zvs_main();

void half_line_uart_main();
void icm42688_main();
void ak09911c_main();
void mmc5983_main();
void smc2025_main();
void lazy_main();

void myservo_main();