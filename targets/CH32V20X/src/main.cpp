#include "sys/core/system.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/gpio/port_virtual.hpp"
#include "sys/debug/debug.hpp"

#include "testbench/tb.h"
#include "app/gui/gui.hpp"
#include "app/gxmatch/gxmatch.hpp"
#include "app/eyetrack/etk.hpp"
#include "app/digiPW/digipw.hpp"


int main(){
    sys::preinit();
    #ifdef CH32V20X
    // eeprom_main();
    // while(true);
    //  uart_main();
    // qlz_main();
    // embd_main();
    // i2c_scanner_main();
    ads1115_main();
    // embd_main();
    // gui_main();
    // canopen_main();
    // bldc_main();
    // digipw_main();
    // sincos_pwm_main();
    // stepper_tb(uart1);
    // co_ab_main();
    // uart_main();
    // rpc_main();
    // leetcode_main();
    // co_ab_main();
    // mpu6050_main();
    // ina3221_main();

    // i2c_scanner_main();
    // tca9548_main();
    // eeprom_main();
    // flash_main();
    // digipw_main();
    // uart_main();
    // wlsy_main();
    // lt8920_main();
    // ma730_main();
    // bmi160_main();
    // delay(200);
    // pmdc_tb();
    // uart_tb(uart1);
    // joystick_tb(uart1);
    // gpio_tb(portC[13]);
    // can_tb(logger, can1, false);
    // m3508_main();
    // stepper_tb(uart1);
    // pmw3901_main();
    // math_tb(uart1);
    // zdt_main(uart1);
    // m3508_main();
    // while(true);
    // lds14_tb(logger, uart2);
    // pca_tb(logger);
    // lua_tb(uart1);p
    // pwm_tb(logger);
    // adc_tb(logger);
    // temp_tb(logger);
    // temp_tb2(logger);
    // eeprom_tb(uart1);
    // eeprom_main();

    while(true);
    #elif defined(CH32V30X)
    // programmer_main();
    // DEBUG_PRINTLN("??");
    // gui_main();
    // math_tb(uart2);

    // curve_tb();

    // math_tb(uart2);
    // gpio_tb(UART7_TX_GPIO);
    // eeprom_main();
    // uart_main();
    // calc_main();
    // dshot_main();
    // usbcdc_tb();
    // embd_main();
    gxm::host_main();

    // pca_main();
    // w25qxx_main();
    // smc_main();
    // can_tb(logger, can1, true);
    // pmdc_tb();
    // gpio_tb(portC[13]);
    // dma_tb(logger, dma1Ch1);
    // dma_tb(logger, UART7_RX_DMA_CH);
    // uart_tb(logger);
    // pmw3901_main();
    // ch9141_tb(logger, uart7);
    // st77xx_tb(spi2);
    // embd_main();
    // mpu6050_main();
    gui_main();
    // sdc_main();
    // proxy_tb();
    // paj7620_main();
    // can_ring_main();
    // m2006_main();
    // gxm::host_main();
    // can_tb(logger, can1);
    while(true);
    #endif
    while(true);
}