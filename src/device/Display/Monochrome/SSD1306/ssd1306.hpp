#ifndef __SSD1306_HPP__
#define __SSD1306_HPP__

#include "device_defs.h"
#include "types/rgb.h"

class DisplayerInterface{
public:
    virtual void init() = 0;
    virtual void writeCommand(const uint8_t cmd) = 0;
    virtual void writeData(const uint8_t data) = 0;
    virtual void writePool(uint8_t * data_ptr, const size_t & len) = 0;
    virtual void writePool(const uint8_t & data, const size_t & len) = 0;
};

#include <optional>

class DisplayInterfaceSpi:public DisplayerInterface{
protected:
    SpiDrv & bus_drv;
    GpioConcept & dc_gpio;
    GpioConcept & res_gpio;
    GpioConcept & blk_gpio;

    static constexpr bool command_level = false;
    static constexpr bool data_level = true;
public:
    DisplayInterfaceSpi(
            SpiDrv & _bus_drv, 
            GpioConcept & _dc_gpio, 
            GpioConcept & _res_gpio = GpioNull,
            GpioConcept & _blk_gpio = GpioNull
            ):bus_drv(_bus_drv),dc_gpio(_dc_gpio), res_gpio(_res_gpio), blk_gpio(_blk_gpio){};
    
    void init() override{
        dc_gpio.OutPP();
        res_gpio.OutPP();
        blk_gpio.OutPP();
    }

    __fast_inline void writeCommand(const uint8_t cmd) override{
        dc_gpio = command_level;
        bus_drv.write(cmd);
    }

    __fast_inline void writeData(const uint8_t data) override{
        dc_gpio = data_level;
        bus_drv.write(data);
    }

    void writePool(uint8_t * data_ptr, const size_t & len) override{
        dc_gpio = data_level;
        bus_drv.write(data_ptr, len);
    }

    void writePool(const uint8_t & data, const size_t & len) override{
        dc_gpio = data_level;
        bus_drv.write(data, len);
    }   
};

class DisplayInterfaceI2c:public DisplayerInterface{
protected:
    I2cDrv bus_drv;
public:
    DisplayInterfaceI2c(I2c & i2c_bus, const uint8_t & i2c_id):bus_drv(i2c_bus, i2c_id){};
};

class OledInterfaceI2c:public DisplayInterfaceI2c{
protected:
    static constexpr uint8_t cmd_token = 0x00;
    static constexpr uint8_t data_token = 0x40;

public:
    OledInterfaceI2c(I2c & i2c_bus, const uint8_t & i2c_id):DisplayInterfaceI2c(i2c_bus, i2c_id){};

    void init()override{;}

    __fast_inline void writeCommand(const uint8_t cmd) override{
        bus_drv.write({cmd_token, cmd});
    }

    __fast_inline void writeData(const uint8_t data) override{
        bus_drv.write({data_token, data});
    }

    void writePool(uint8_t * data_ptr, const size_t & len) override{
        bus_drv.write(data_token, false);
        bus_drv.write(data_ptr, len);
    }

    void writePool(const uint8_t & data, const size_t & len) override{
        bus_drv.write(data_token, false);
        bus_drv.write(data, len);
    }    
};


class SSD1306{
protected:
    DisplayerInterface & interface;

    uint16_t width = 72;
    uint16_t height = 40;
    uint16_t x_offset = 0;



    void setPos(uint16_t x,uint16_t y){
        x += x_offset;
        y >>= 3;
        interface.writeCommand(0xb0 + y);
        interface.writeCommand(((x&0xf0)>>4)|0x10);
        interface.writeCommand((x&0x0f));
    }
public:
    static constexpr uint8_t default_id = 0x78;

    SSD1306(DisplayerInterface & _interface):interface(_interface){;}
    void init();
    void flush(const Binary & color);     

    void setOffsetX(const uint8_t & offset){x_offset = offset;}
    void setOffsetY(const uint8_t & offset){
        interface.writeCommand(0xD3); 
        interface.writeCommand(offset);
    }

    void enableFlipY(const bool & flip = true){interface.writeCommand(0xA0 | flip);}
    void enableFlipX(const bool & flip = true){interface.writeCommand(0xC0 | (flip << 3));}
    void enableInversion(const bool & inv = true){interface.writeCommand(0x7A - inv);}  
};


#endif