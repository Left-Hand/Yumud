#include "TM7705.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Error = TM7705::Error;

template<typename T = void>    
using IResult = Result<T, Error>;


IResult<> TM7705_Phy::write_byte(uint8_t dat1){	//将数据dat1写入TM7705
    sclk_gpio_ = HIGH;
    for(uint8_t i=0;i<8;i++){
        sclk_gpio_= LOW;
        clock::delay(1us);
        if(dat1&0x80) din_gpio_ = HIGH;	   //先写高位
        else        din_gpio_= LOW;
        clock::delay(1us);
        sclk_gpio_ = HIGH;
        clock::delay(1us);
        dat1 <<= 1;
    }
    sclk_gpio_ = HIGH;
    din_gpio_ = HIGH;

    return Ok();
}


IResult<> TM7705_Phy::read_byte(uint8_t & ret){   //从TM7705寄存器读数据
    uint8_t i,dat2=0;
    sclk_gpio_ = HIGH;
    for(i=0;i<8;i++)
    {
        sclk_gpio_= LOW;
        clock::delay(1us);
        dat2=(dat2<<1) | bool(dout_gpio_.read());	  // 高位先读
        clock::delay(1us);
        sclk_gpio_ = HIGH;
        clock::delay(1us);
    }
    sclk_gpio_ = HIGH;
    ret = dat2;

    return Ok();
}


IResult<> TM7705_Phy::init(){
    sclk_gpio_.outpp(HIGH);
    din_gpio_.outpp(HIGH);
    dout_gpio_ = HIGH;
    drdy_gpio_ = HIGH;
    
    for(uint8_t i=0;i<40;i++){
        sclk_gpio_= LOW;
        clock::delay(1us);
        sclk_gpio_ = HIGH;
        clock::delay(1us);
    }

    return Ok();
}
