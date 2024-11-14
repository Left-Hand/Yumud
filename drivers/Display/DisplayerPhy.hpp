#pragma once

#include "drivers/device_defs.h"
#include "types/rgb.h"

#include <optional>

namespace ymd::drivers{

class DisplayerPhy{
public:
    virtual void init() = 0;

    virtual void writeCommand(const uint32_t cmd) = 0;
    virtual void writeData(const uint32_t data) = 0;

    virtual void writeU8(const uint8_t data, size_t len) = 0;
    virtual void writeU8(const uint8_t * data, size_t len) = 0;
};

class DisplayerPhySpi:public DisplayerPhy{
protected:
// public:
    SpiDrv spi_drv_;
    GpioConcept & dc_gpio;
    GpioConcept & res_gpio;
    GpioConcept & blk_gpio;

    static constexpr bool command_level = false;
    static constexpr bool data_level = true;

    // DisplayerPhySpi(const DisplayerPhySpi & other) = default;
    // DisplayerPhySpi(DisplayerPhySpi && other) = default;
public:

    DisplayerPhySpi(
            SpiDrv && sou_drv, 
            GpioConcept & _dc_gpio, 
            GpioConcept & _res_gpio = GpioNull,
            GpioConcept & _blk_gpio = GpioNull
        ) :spi_drv_(std::move(sou_drv)), dc_gpio(_dc_gpio), res_gpio(_res_gpio), blk_gpio(_blk_gpio){}

    // 拷贝构造函数
    DisplayerPhySpi(
        const SpiDrv & sou_drv, 
        GpioConcept & _dc_gpio, 
        GpioConcept & _res_gpio = GpioNull,
        GpioConcept & _blk_gpio = GpioNull
    ) : spi_drv_(sou_drv), dc_gpio(_dc_gpio), res_gpio(_res_gpio), blk_gpio(_blk_gpio){}


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

    void writeCommand(const uint32_t cmd) override{
        dc_gpio = command_level;
        spi_drv_.writeSingle<uint8_t>(cmd);
    }

    void writeData(const uint32_t data) override{
        dc_gpio = data_level;
        spi_drv_.writeSingle<uint8_t>(data);
    }

    void writeData16(const uint32_t data){
        dc_gpio = data_level;
        spi_drv_.writeSingle<uint16_t>(data);
    }

    void writeSingle(const auto & data){
        dc_gpio = data_level;
        spi_drv_.writeSingle(data);
    }

    template<typename U>
    void writeMulti(const auto * data, size_t len){
        dc_gpio = data_level;
        spi_drv_.writeMulti<U>(data, len);
    }

    template<typename U>
    void writeMulti(const auto & data, size_t len){
        dc_gpio = data_level;
        spi_drv_.writeMulti<U>(data, len);
    }


    void writeU8(const uint8_t data, size_t len) override{
        dc_gpio = data_level;
        spi_drv_.writeMulti<uint8_t>(data, len);
    }

    void writeU8(const uint8_t * data, size_t len) override{
        dc_gpio = data_level;
        spi_drv_.writeMulti<uint8_t>(data, len);
    }
};


class DisplayerPhyI2c:public DisplayerPhy{
protected:
    I2cDrv i2c_drv_;
    static constexpr uint8_t cmd_token = 0x00;
    static constexpr uint8_t data_token = 0x40;
    static constexpr uint8_t default_i2c_addr = 0x78;
public:
    DisplayerPhyI2c(const I2cDrv & i2c_drv):i2c_drv_(i2c_drv){};
    DisplayerPhyI2c(I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){};
    DisplayerPhyI2c(I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(I2cDrv{i2c, i2c_addr}){};

    void init()override{;}

    void writeCommand(const uint32_t cmd) override{
        i2c_drv_.writeReg(cmd_token, cmd, LSB);
    }

    void writeData(const uint32_t data) override{
        i2c_drv_.writeReg(data_token, data, LSB);
    }

    void writeMulti(const is_stdlayout auto * data_ptr, size_t len){
        i2c_drv_.writeMulti(data_token, data_ptr, len, LSB);
    }

    void writeMulti(const is_stdlayout auto data, size_t len){
        i2c_drv_.writeMulti(data_token, data, len, LSB);
    }

    void writeU8(const uint8_t data, size_t len) override{
        writeMulti<uint8_t>(data, len);
    }

    void writeU8(const uint8_t * data, size_t len) override{
        writeMulti<uint8_t>(data, len);
    }
};

};