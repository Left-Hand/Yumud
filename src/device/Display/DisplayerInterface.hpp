#ifndef __DISPLAYERINTERFACE_HPP__
#define __DISPLAYERINTERFACE_HPP__


#include "device_defs.h"
#include "types/rgb.h"
#include <optional>

class DisplayerInterface{
public:
    virtual void init() = 0;
    virtual void writeCommand(const uint8_t & cmd) = 0;


    virtual void writeData(const uint8_t & data) = 0;
    virtual void writePool(const uint8_t * data_ptr, const size_t & len) = 0;
    virtual void writePool(const uint8_t & data, const size_t & len) = 0;
};


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
        res_gpio.OutPP(1);
        blk_gpio.OutPP(1);

        reset();
    }

    void reset(){
        res_gpio.clr();
        delay(1);
        res_gpio.set();
    }

    void setBackLight(const uint8_t & brightness){

    }

    __fast_inline void writeCommand(const uint8_t & cmd) override{
        dc_gpio = command_level;
        bus_drv.write(cmd);
    }


    __fast_inline void writeData(const uint8_t & data) override{
        dc_gpio = data_level;
        bus_drv.write(data);
    }

    void writePool(const uint8_t * data_ptr, const size_t & len) override{
        dc_gpio = data_level;
        bus_drv.write(data_ptr, len);
    }


    void writePool(const uint8_t & data, const size_t & len) override{
        dc_gpio = data_level;
        bus_drv.write(data, len);
    }

    __fast_inline void writeData(const uint16_t & data){
        dc_gpio = data_level;
        bus_drv.write(data);
    }

    void writePool(const uint16_t * data_ptr, const size_t & len){
        dc_gpio = data_level;
        bus_drv.write(data_ptr, len);
    }


    void writePool(const uint16_t & data, const size_t & len){
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

    __fast_inline void writeCommand(const uint8_t & cmd) override{
        bus_drv.writeReg(cmd_token, cmd);
    }

    __fast_inline void writeData(const uint8_t & data) override{
        bus_drv.writeReg(data_token, data);
    }

    void writePool(const uint8_t * data_ptr, const size_t & len) override{
        bus_drv.writePool(data_token, data_ptr, 1, len, false);
    }

    void writePool(const uint8_t & data, const size_t & len) override{
        // bus_drv.write(data_token, false);
        auto data_ptr = new uint8_t[len];
        memset(data_ptr, data, len);
        bus_drv.writePool(data_token, data_ptr, 1, len, false);
        // bus_drv.write(data, len);
        delete data_ptr;
    }
};


#endif /* DISPLAYERINTERFACE_HPP */
