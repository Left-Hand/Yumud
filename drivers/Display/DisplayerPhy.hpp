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
    hal::SpiDrv spi_drv_;
    hal::GpioIntf & dc_gpio;
    hal::GpioIntf & res_gpio;
    hal::GpioIntf & blk_gpio;

    static constexpr bool command_level = false;
    static constexpr bool data_level = true;

    // DisplayerPhySpi(const DisplayerPhyhal::Spi & other) = default;
    // DisplayerPhySpi(DisplayerPhyhal::Spi && other) = default;
public:

    DisplayerPhySpi(
            hal::SpiDrv && spi_drv, 
            hal::GpioIntf & _dc_gpio, 
            hal::GpioIntf & _res_gpio = hal::NullGpio,
            hal::GpioIntf & _blk_gpio = hal::NullGpio
        ) :spi_drv_(std::move(spi_drv)), dc_gpio(_dc_gpio), res_gpio(_res_gpio), blk_gpio(_blk_gpio){}

    // 拷贝构造函数
    DisplayerPhySpi(
        const hal::SpiDrv & spi_drv, 
        hal::GpioIntf & _dc_gpio, 
        hal::GpioIntf & _res_gpio = hal::NullGpio,
        hal::GpioIntf & _blk_gpio = hal::NullGpio
    ) : spi_drv_(spi_drv), dc_gpio(_dc_gpio), res_gpio(_res_gpio), blk_gpio(_blk_gpio){}


    DisplayerPhySpi(
            hal::Spi & _bus,
            const uint8_t index,
            hal::GpioIntf & _dc_gpio, 
            hal::GpioIntf & _res_gpio = hal::NullGpio,
            hal::GpioIntf & _blk_gpio = hal::NullGpio
            ):DisplayerPhySpi(hal::SpiDrv(_bus, index), _dc_gpio, _res_gpio, _blk_gpio) {};

    void init() override{
        dc_gpio.outpp();
        res_gpio.outpp(HIGH);
        blk_gpio.outpp(HIGH);

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
    hal::I2cDrv i2c_drv_;
    static constexpr uint8_t cmd_token = 0x00;
    static constexpr uint8_t data_token = 0x40;
    static constexpr uint8_t default_i2c_addr = 0x78;
public:
    DisplayerPhyI2c(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){};
    DisplayerPhyI2c(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){};
    DisplayerPhyI2c(hal::I2c & i2c, const uint8_t i2c_addr = default_i2c_addr):i2c_drv_(hal::I2cDrv{i2c, i2c_addr}){};

    void init()override{;}

    void writeCommand(const uint32_t cmd) override{
        i2c_drv_.writeReg(cmd_token, cmd, LSB);
    }

    void writeData(const uint32_t data) override{
        i2c_drv_.writeReg(data_token, data, LSB);
    }

    void writeMulti(const is_stdlayout auto * pdata, size_t len){
        if constexpr(sizeof(*pdata) != 1){
            i2c_drv_.writeMulti(data_token, std::span(pdata, len), LSB);
        }else {
            i2c_drv_.writeMulti(data_token, std::span(pdata, len));
        }
    }

    void writeMulti(const is_stdlayout auto data, size_t len){
        if constexpr(sizeof(data) != 1){
            i2c_drv_.writeSame(data_token, std::span(data, len), LSB);
        }else {
            i2c_drv_.writeSame(data_token, data, len);
        }
    }

    void writeU8(const uint8_t data, size_t len) override{
        writeMulti<uint8_t>(data, len);
    }

    void writeU8(const uint8_t * data, size_t len) override{
        writeMulti<uint8_t>(data, len);
    }
};

};