#include "tb.h"

#include "hal/bus/spi/spisw.hpp"
#include "drivers/HID/ps2_joystick/ps2_joystick.hpp"


#define JOYSTICK_TB

void PS2_Cmd(u8 CMD) 
{
  volatile u16 ref=0x01; 
//   Data[1]=0; 
  for(ref=0x01;ref<0x0100;ref<<=1) 
  {
    if(ref&CMD) 
    {
    //  DO_H; // 输出一位控制位
    SPI1_MOSI_Gpio = 1;
    }
    // else DO_L; 
    SPI1_MOSI_Gpio = 0;
    // CLK_H; // 时钟拉高 

    SPI1_SCLK_Gpio = 1;
    delayMicroseconds(10); 
    SPI1_SCLK_Gpio = 0;
    delayMicroseconds(10); 
    SPI1_SCLK_Gpio = 1;
    // if(SPI1_MISO_Gpio) 
    // {Data[1]=ref|Data[1];}
  }
 delayMicroseconds(16);
}

// 判断是否为红灯模式，0x41=模拟绿灯，0x73=模拟红灯 
// 返回值；0，红灯模式 
// 其他，其他模式
u8 PS2_RedLight(void) 
{
  SPI1_CS_Gpio = 0;
  PS2_Cmd(0x01); // 开始命令
  PS2_Cmd(0x42); // 请求数据
  SPI1_CS_Gpio = 1;
//   if( Data[1]== 0X73) return 0;
  return 0;
}


void joystick_tb(OutputStream & logger){
    #ifdef JOYSTICK_TB

    using Event = Ps2Joystick::JoyStickEvent;

    // spi1.init(1000000);
    spi1.bindCsPin(portA[15], 0);

    SpiSw spisw{SPI1_SCLK_Gpio, SPI1_MOSI_Gpio, SPI1_MISO_Gpio, SPI1_CS_Gpio};
    auto & spi = spisw;
    spi.init(100000);
    spi.configBitOrder(false);

    SpiDrv ps2_drv{spi, 0};
    Ps2Joystick joystick{ps2_drv};
    joystick.init();


    // while(true){
    //     // SPI1_CS_Gpio = !SPI1_CS_Gpio;
    //     PS2_RedLight();
    //     delay(10);
    // }

    // uint32_t d = 0;
    // while(true){
    //     uint32_t r;
    //     d = (d+1) % 256;
    //     spi.transfer(r, uint32_t(d));
    //     logger.println(d, r);
    //     delay(10);
    // }
    while(true){
        // SPI1_MISO_Gpio = !SPI1_MISO_Gpio;
        // logger.println("??");
        joystick.update();
        logger.println(joystick.frame.data[0], joystick.frame.data[1], (uint8_t)joystick.id(), joystick.permit);
        delay(10);
        // delay(100);
            // (uint8_t)joystick.id(), joystick.valueof(Event::LX));
    }
    #endif
}