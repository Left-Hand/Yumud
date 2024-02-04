#ifndef __SSD1306_HPP__
#define __SSD1306_HPP__

#include "../bus/i2c/i2c.hpp"
#include "../bus/spi/spi2.hpp"
#include "../bus/busdrv.hpp"
#include "../../types/rgb.h"

#define SSD1306_DC_Port SPI2_Port
#define SSD1306_DC_Pin SPI2_MISO_Pin

#define SSD1306_ON_DATA \
SSD1306_DC_Port -> BSHR = SSD1306_DC_Pin;

#define SSD1306_ON_CMD \
SSD1306_DC_Port -> BCR = SSD1306_DC_Pin;

class SSD1306{
private:
    BusDrv & busdrv;

    uint16_t w = 128;
    uint16_t h = 64;
    uint16_t x_offset = 0;

    const uint8_t cmd_token = 0x00;
    const uint8_t data_token = 0x40;

    __fast_inline void writeCommand(const uint8_t cmd){

        if(busdrv.isSpiBus()){
            SSD1306_ON_CMD;
            busdrv.write(cmd);
        }else if(busdrv.isI2cBus()){
            busdrv.write({cmd_token, cmd});
        }
    }

    __fast_inline void writeData(const uint8_t data){
        if(busdrv.isSpiBus()){
            SSD1306_ON_DATA;
            busdrv.write(data);
        }else if(busdrv.isI2cBus()){
            busdrv.write({data_token, data});
        }
    }

    void writePool(uint8_t * data_ptr, const size_t & len){
        if(busdrv.isSpiBus()){
            SSD1306_ON_DATA;
            busdrv.write(data_ptr, len);
        }else if(busdrv.isI2cBus()){
            busdrv.write(data_token, false);
            busdrv.write(data_ptr, len);
        }
    }

    void writePool(const uint8_t & data, const size_t & len){
        if(busdrv.isSpiBus()){
            SSD1306_ON_DATA;
            busdrv.write(data, len);
        }else if(busdrv.isI2cBus()){
            busdrv.write(data_token, false);
            busdrv.write(data, len);
        }
    }

    void setPos(uint16_t x,uint16_t y){
        x += x_offset;
        y >>= 3;
        writeCommand(0xb0 + y);
        writeCommand(((x&0xf0)>>4)|0x10);
        writeCommand((x&0x0f));
    }
public:
    SSD1306(BusDrv & _busdrv):busdrv(_busdrv){;}
    void init();
    void flush(bool color);     

    void setOffsetX(const uint8_t offset){x_offset = offset;}
    void setOffsetY(const uint8_t offset){writeCommand(0xD3); writeCommand(offset);}
    void setFlipY(const bool & flip){writeCommand(0xA0 | flip);}
    void setFlipX(const bool & flip){writeCommand(0xC0 | (flip << 3));}
    void setInversion(const bool & inv){writeCommand(0x7A - inv);}  
};


#endif