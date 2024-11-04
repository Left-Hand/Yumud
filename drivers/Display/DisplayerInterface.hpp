#pragma once

#include "../drivers/device_defs.h"
#include "../types/rgb.h"

#include <optional>

namespace yumud::drivers{

class DisplayerPhy{
public:
    virtual void init() = 0;

    virtual void writeCommand(const uint8_t cmd) = 0;
    virtual void writeData(const uint8_t data) = 0;

    virtual void writePool(const uint8_t * data_ptr, const size_t len) = 0;
    virtual void writePool(const uint8_t data, const size_t len) = 0;
};

class DisplayerPhySpi:public DisplayerPhy{
protected:
// public:
    SpiDrv spi_drv;
    GpioConcept & dc_gpio;
    GpioConcept & res_gpio;
    GpioConcept & blk_gpio;

    static constexpr bool command_level = false;
    static constexpr bool data_level = true;

    DisplayerPhySpi(const DisplayerPhySpi & other) = default;
    DisplayerPhySpi(DisplayerPhySpi && other) = default;
public:

    DisplayerPhySpi(
            SpiDrv && _bus_drv, 
            GpioConcept & _dc_gpio, 
            GpioConcept & _res_gpio = GpioNull,
            GpioConcept & _blk_gpio = GpioNull
        ) :spi_drv(std::move(_bus_drv)), dc_gpio(_dc_gpio), res_gpio(_res_gpio), blk_gpio(_blk_gpio){}

    // 拷贝构造函数
    DisplayerPhySpi(
        const SpiDrv & _bus_drv, 
        GpioConcept & _dc_gpio, 
        GpioConcept & _res_gpio = GpioNull,
        GpioConcept & _blk_gpio = GpioNull
    ) : spi_drv(_bus_drv), dc_gpio(_dc_gpio), res_gpio(_res_gpio), blk_gpio(_blk_gpio){}


    DisplayerPhySpi(
            Spi & _bus,
            const uint8_t index,
            GpioConcept & _dc_gpio, 
            GpioConcept & _res_gpio = GpioNull,
            GpioConcept & _blk_gpio = GpioNull
            ):DisplayerPhySpi(SpiDrv(_bus, index), _dc_gpio, _res_gpio, _blk_gpio) {};

    void init() override{
        dc_gpio.outpp();
        res_gpio.outpp(1);
        blk_gpio.outpp(1);

        reset();
    }

    void reset(){
        delay(10);
        res_gpio.clr();
        delay(10);
        res_gpio.set();
    }

    void setBackLight(const uint8_t brightness){

    }

    void writeCommand(const uint8_t cmd) override{
        dc_gpio = command_level;
        spi_drv.write(cmd);
    }

    void writeData(const uint8_t data) override{
        dc_gpio = data_level;
        spi_drv.write(data);
    }

    void writePool(const uint8_t * data_ptr, const size_t len) override{
        dc_gpio = data_level;
        spi_drv.write(data_ptr, len);
    }


    void writePool(const uint8_t data, const size_t len) override{
        dc_gpio = data_level;
        spi_drv.write(data, len);
    }

    void writeData(const uint16_t data){
        dc_gpio = data_level;
        spi_drv.write(data);
    }

    void writePool(const uint16_t * data_ptr, const size_t len){
        dc_gpio = data_level;
        spi_drv.write(data_ptr, len);
    }


    void writePool(const uint16_t data, const size_t len){
        dc_gpio = data_level;
        spi_drv.write(data, len);
    } 


    template<is_rgb T>
    void writePixels(const T * data, const size_t len){
        dc_gpio = data_level;
        spi_drv.write<uint8_t, Grayscale, RGB565>((const uint8_t *)data, len);
    } 
};

class DisplayInterfaceI2c:public DisplayerPhy{
protected:
    I2cDrv bus_drv;
public:
    DisplayInterfaceI2c(I2c & i2c_bus, const uint8_t i2c_id):bus_drv(i2c_bus, i2c_id){};
};

class DisplayerPhyI2cc:public DisplayInterfaceI2c{
protected:
    static constexpr uint8_t cmd_token = 0x00;
    static constexpr uint8_t data_token = 0x40;
    static constexpr uint8_t oled_default_addr = 0x78;
public:
    DisplayerPhyI2cc(I2c & i2c_bus, const uint8_t i2c_id = oled_default_addr):DisplayInterfaceI2c(i2c_bus, i2c_id){};

    void init()override{;}

    void writeCommand(const uint8_t cmd) override{
        bus_drv.writeReg(cmd_token, cmd, LSB);
    }

    void writeData(const uint8_t data) override{
        bus_drv.writeReg(data_token, data, LSB);
    }

    void writePool(const uint8_t * data_ptr, const size_t len) override{
        bus_drv.writePool(data_token, data_ptr, len, LSB);
    }

    void writePool(const uint8_t data, const size_t len) override{
        bus_drv.writePool(data_token, data, len, LSB);
    }
};

};