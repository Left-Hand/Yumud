#include "core/system.hpp"

#include "hal/bus/spi/spihw.hpp"
#include "hal/gpio/vport.hpp"
#include "core/debug/debug.hpp"

#include "testbench/tb.h"


int main(){
    ymd::sys::preinit();
    #ifdef CH32V20X
    // myesc_main();
    sincos_main();
    // sincospll_main();

    // vce2755_main();
    // sincospll_main();
    // myactuator_main();
    // eeprom_main();
    // while(true);
    //  uart_main();
    //  can_ring_main();
    // qlz_main();
    // embd_main();
    // ina3221_main();
    // mpu6050_main();
    // vl6180x_main();
    // svpwm3_main();
    // sincos_pwm_main();
    // pfm_zvs_main();
    // phase3pwm_main();
    // half_line_uart_main();
    // rrs3_robot_main();
    // oled_main();
    // i2c_scanner_main();
    // oled_main();
    // light_tracking_main();
    // tm1637_main();
    // ads7830_main();
    // polar_robot_main();
    // bldc_main();
    // kth7823_main();
    // joystick_main();
    // icm42688_main();
    // mmc5983_main();
    // i2c_scanner_main();
    // bldc_main();
    // st1615_main();
    // bldc_main();
    // nuedc_2025e_laser_main();
    // ft6336_main();
    // polar_robot_main();
    // i2c_scanner_main();
    // rrs3_robot_main();
    // can_ring_main();
    // aw9523_main();

    // ads1115_main();
    // embd_main();
    // gui_main();
    // canopen_main();
    // bldc_main();
    // myesc_main();
    // alx_aoa_main();
    // mk8000tr_main();
    // vl53l5cx_main();

    // adrc_main();
    // mystepper_main();

    // nuedc_2025e_main();
    // polar_robot_main();
    // mks_stepper_main();
    // ads7830_main();
    // render_main();
    // math_main();
    // polar_robot_main();
    // nuedc_2025e_main();
    // render_main();
    // bldc_main();
    // laser_ctl_main();

    // bldc_main();
    // laser_ctl_main();
    // diffspd_vehicle_main();
    // mystepper_main();
    // diffspd_vehicle_main();
    // ws2812_main();
    // digipw_main();
    // enum_main();
    // hive_main();
    // rmst_main();
    // dsp_main();
    
    // fmt_main();
    // sincos_pwm_main();
    // stepper_tb(usart1);
    // co_ab_main();
    // uart_main();
    // rpc_main();
    // leetcode_main();
    // co_ab_main();
    // mpu6050_main();
    // i2c_scanner_main();
    // lt8960_main();
    // ws2812_main();
    // gxm_new_energy_main();
    // rrs3_robot_main();
    // nuedc_2023e_main();
    // icm42688_main();
    // mmc5983_main();
    // lazy_main();
    // ak09911c_main();
    // enum_main();
    // i2c_scanner_main();
    // pmw3901_main();
    // light_tracking_main();
    // coro_main();

    // i2c_scanner_main();


    // i2c_scanner_main();
    // tca9548_main();
    // eeprom_main();
    // ht16k33_main();
    // mystepper_main();
    // zdt_main();
    // polar_robot_main();
    // ads7830_main();
    dsp_main();
    // can_ring_main();
    // rrs3_robot_main();
    // flash_main();
    // digipw_main();
    // uart_main();
    // wlsy_main();
    // lt8920_main();
    // ma730_main();
    // bmi160_main();
    // clock::delay(200ms);
    // pmdc_tb();
    // uart_tb(usart1);
    // joystick_tb(usart1);
    // gpio_tb(hal::PC<13>());
    // can_tb(logger, can1, false);
    // m3508_main();
    // stepper_tb(usart1);
    // pmw3901_main();
    // math_tb(usart1);
    // zdt_main(usart1);
    // m3508_main();
    // while(true);
    // lds14_tb(logger, usart2);
    // pca_tb(logger);
    // lua_tb(usart1);p
    // pwm_tb(logger);
    // adc_tb(logger);
    // temp_tb(logger);
    // temp_tb2(logger);
    // eeprom_tb(usart1);
    // eeprom_main();
    // myservo_main();
    // myesc_main();
    // bldc_main();
    while(true);
    #elif defined(CH32V30X)
    // sincos_main();

    // myesc_main();
    // s21c_main();
    // steadywin_main();
    // alx_aoa_main();
    smc2025_main();
    // vl53l5cx_main();
    // smc2025_main();
    // m1502e_main();

    // m10_main();
    // alx_aoa_main();

    // icm42688_main();
    // bmi160_main();
    // i2c_scanner_main();
    // ld19_main();
    // i2c_scanner_main();
    // slcan_main();
    // programmer_main();
    // gui_main();
    // math_tb(usart2);

    // curve_tb();

    // math_tb(hal::usart2);
    // gpio_tb(UART7_TX_GPIO);
    // eeprom_main();
    // uart_main();
    // render_main();
    // calc_main();
    // dshot_main();
    // usbcdc_tb();
    // embd_main();
    // gxm::host_main();

    // pca_main();
    // w25qxx_main();
    // smc_main();
    // can_tb(logger, can1, true);
    // pmdc_tb();
    // gpio_tb(hal::PC<13>());
    // dma_tb(logger, dma1_ch1);
    // dma_tb(logger, UART7_RX_DMA_CH);
    // uart_tb(logger);
    // pmw3901_main();
    // ch9141_tb(logger, uart7);
    // st7789_main();
    // smc2025_main();
    mlx90640_main();
    // fft_main();
    // adrc_main();
    // render_main();
    // script_main();
    // svd_main();
    // mlx90640_main();
    // smc2025_main();
    
    // slcan_main();

    // embd_main();
    // mpu6050_main();
    // gui_main();
    // light_tracking_main();
    // cubic_main();
    // smc2025_main();
    // st1615_main();

    
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