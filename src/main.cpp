#include "misc.h"
#include "apps.h"

using Complex = Complex_t<real_t>;
using Color = Color_t<real_t>;

#include "dsp/pll.hpp"
#include "src/opa/opa.hpp"
#include "src/device/Adc/INA226/ina226.hpp"

#define I2C_BaudRate 400000


#include "src/device/Encoder/Estimmator.hpp"
#include "src/device/Memory/EEPROM/AT24CXX/at24c02.hpp"

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
// 
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



using Sys::t;

// static Printer & logger = uart2;
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

#include "src/device/Encoder/ABEncoder.hpp"
#include "src/device/Encoder/OdometerLines.hpp"
#include "src/adc/adcs/adc1.hpp"
#include "dsp/oscillator.hpp"
#include "src/device/Dac/TM8211/tm8211.hpp"

#include "json.hpp"

#include <bits/stl_numeric.h>
#include <optional>

#include "src/bus/bus_inc.h"
#include "robots/buck.hpp"
#include "robots/stepper.hpp"


template<typename T>
struct targAndMeasurePair_t{
    T target;
    T measure;
};

using targAndMeasurePair = targAndMeasurePair_t<real_t>;


real_t warp_to_halfpi(const real_t & x){
    real_t ret = x;
    while(ret > PI / 2) ret -= PI;
    while(ret < -PI / 2) ret += PI;
    return ret;
}


struct buckRuntimeValues{
    targAndMeasurePair curr;
    targAndMeasurePair volt;
};





// void DMA_INIT(void){
//     dma1Ch4.init(DmaChannel::Mode::toPeriph);
//     dma1Ch4.begin((void *)(&USART1->DATAR), (void *)mem_buf, mem_size);
//     dma1Ch4.enableIt({1,1});
//     dma1Ch4.enableDoneIt();

//     DMA_Cmd (DMA1_Channel4,ENABLE);  //使能DMA
// }

// void DMA_RX_INIT(void){
//     auto & rxDma = UART2_RX_DMA_CH;
//     rxDma.init(DmaChannel::Mode::toMemCircular);
//     rxDma.enableIt({1,1});
//     rxDma.enableDoneIt();
//     rxDma.enableHalfIt();
//     rxDma.begin((void *)mem_buf,(void *)(&USART2->DATAR), mem_size);
    // rxDma.begin((void *)u1rx_dma_buf, (void *)(&instance->DATAR), rx_dma_buf_size);
    // DMA_Cmd (DMA1_Channel4,ENABLE);  //使能DMA
// }


int main(){

    Sys::Misc::prework();
    dma1Ch6.begin();
    UartHw & uart = uart2;
    // uart.init(115200 * 8, CommMethod::Dma, CommMethod::Dma);
    uart.init(115200 * 8, CommMethod::Dma, CommMethod::Dma);
    // uart.init(115200 * 8, CommMethod::Dma, CommMethod::Blocking);
    // uart.init(115200 * 8);
    // DEBUG_PRINT(dma1Ch4.dma_index, dma1Ch4.channel_index);
    // DMA_INIT();
    // USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    // delay(100);
    // mem_buf[5] = '0';
    // DMA_INIT();
    // USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    // delay(100);
    // while(true);
    // uart1.init(115200 * 8, CommMethod::Interrupt, CommMethod::Dma);
    // test();
    // uint8_t cnt = 0;

    // while(true){
    //     if(uart.available()){
    //         delay(10);
    //         uart.println(uart.readStringAll());
    //     }
    //     delay(500);
    //     uart.println("??");
    // }

    // DMA_RX_INIT();

    while(true){
        delay(100);
        // for(uint8_t i = 0; i < 8; i++)
        // uart.txBuf.addData('.');
        // uart << uart.txBuf.straight() << "\r\n";
        // uart << "abcdefg\r\n";
        // uart.println(millis());

        if(uart.available()){
            delay(10);
            uart.println(uart.readString());
        }
    }

    // Stepper stp;
    // stp.init();
    // while(true){
    //     stp.run();
    // }
    // pedestrian_app();
    // modem_app();
    // test_app();
    // pmdc_test();
    // SpreadCycle::chopper_test();
    // buck_test();
    // osc_test();
}