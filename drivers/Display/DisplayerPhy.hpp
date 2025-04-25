#pragma once

#include "core/io/regs.hpp"
#include "types/rgb.h"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"
#include "hal/gpio/gpio_port.hpp"

#include <optional>

namespace ymd::drivers{

class DisplayerPhyIntf{
public:
    virtual void init() = 0;

    virtual hal::BusError write_command(const uint32_t cmd) = 0;
    virtual hal::BusError write_data(const uint32_t data) = 0;

    virtual void write_u8(const uint8_t data, size_t len) = 0;
    virtual void write_u8(const uint8_t * data, size_t len) = 0;
};

class DisplayerPhySpi final:public DisplayerPhyIntf{
protected:
// public:
    hal::SpiDrv spi_drv_;
    hal::GpioIntf & dc_gpio_;
    hal::GpioIntf & res_gpio_;
    hal::GpioIntf & blk_gpio_;

    static constexpr bool command_level = false;
    static constexpr bool data_level = true;

    // DisplayerPhySpi(const DisplayerPhyhal::Spi & other) = default;
    // DisplayerPhySpi(DisplayerPhyhal::Spi && other) = default;
public:

    DisplayerPhySpi(
            hal::SpiDrv && spi_drv, 
            hal::GpioIntf & dc_gpio, 
            hal::GpioIntf & res_gpio = hal::NullGpio,
            hal::GpioIntf & blk_gpio = hal::NullGpio
        ) :spi_drv_(std::move(spi_drv)), dc_gpio_(dc_gpio), res_gpio_(res_gpio), blk_gpio_(blk_gpio){}

    // 拷贝构造函数
    DisplayerPhySpi(
        const hal::SpiDrv & spi_drv, 
        hal::GpioIntf & dc_gpio, 
        hal::GpioIntf & res_gpio = hal::NullGpio,
        hal::GpioIntf & blk_gpio = hal::NullGpio
    ) : spi_drv_(spi_drv), dc_gpio_(dc_gpio), res_gpio_(res_gpio), blk_gpio_(blk_gpio){}


    DisplayerPhySpi(
        hal::Spi & bus,
        const hal::SpiSlaveIndex index,
        hal::GpioIntf & dc_gpio, 
        hal::GpioIntf & res_gpio = hal::NullGpio,
        hal::GpioIntf & blk_gpio = hal::NullGpio
    ):DisplayerPhySpi(hal::SpiDrv(bus, index), dc_gpio, res_gpio, blk_gpio) {};

    void init(){
        dc_gpio_.outpp();
        res_gpio_.outpp(HIGH);
        blk_gpio_.outpp(HIGH);

        reset();
    }

    void reset(){
        delay(10);
        res_gpio_.clr();
        delay(10);
        res_gpio_.set();
    }

    void set_back_light(const uint8_t brightness){

    }

    hal::BusError write_command(const uint32_t cmd){
        dc_gpio_ = command_level;
        return spi_drv_.write_single<uint8_t>(cmd);
    }

    hal::BusError write_data(const uint32_t data){
        dc_gpio_ = data_level;
        return spi_drv_.write_single<uint8_t>(data);
    }

    void write_data16(const uint32_t data){
        dc_gpio_ = data_level;
        spi_drv_.write_single<uint16_t>(data).unwrap();
    }

    void write_single(const auto & data){
        dc_gpio_ = data_level;
        spi_drv_.write_single(data).unwrap();
    }

    template<typename U>
    void write_burst(const auto * data, size_t len){
        dc_gpio_ = data_level;
        spi_drv_.write_burst<U>(data, len).unwrap();
    }

    template<typename U>
    void write_burst(const auto & data, size_t len){
        dc_gpio_ = data_level;
        spi_drv_.write_burst<U>(data, len).unwrap();
    }


    void write_u8(const uint8_t data, size_t len){
        dc_gpio_ = data_level;
        spi_drv_.write_burst<uint8_t>(data, len).unwrap();
    }

    void write_u8(const uint8_t * data, size_t len){
        dc_gpio_ = data_level;
        spi_drv_.write_burst<uint8_t>(data, len).unwrap();
    }
};


class DisplayerPhyI2c final:public DisplayerPhyIntf{
protected:
    hal::I2cDrv i2c_drv_;
    static constexpr uint8_t CMD_TOKEN = 0x00;
    static constexpr uint8_t DATA_TOKEN = 0x40;
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x78);
public:
    DisplayerPhyI2c(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){};
    DisplayerPhyI2c(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){};
    DisplayerPhyI2c(hal::I2c & i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv{i2c, addr}){};

    void init(){;}

    hal::BusError write_command(const uint32_t cmd){
        return i2c_drv_.write_reg(CMD_TOKEN, cmd, LSB);
    }

    hal::BusError write_data(const uint32_t data){
        return i2c_drv_.write_reg(DATA_TOKEN, data, LSB);
    }

    hal::BusError write_burst(const is_stdlayout auto * pdata, size_t len){
        if constexpr(sizeof(*pdata) != 1){
            return i2c_drv_.write_burst(DATA_TOKEN, std::span(pdata, len), LSB);
        }else {
            return i2c_drv_.write_burst(DATA_TOKEN, std::span(pdata, len));
        }
    }

    hal::BusError write_burst(const is_stdlayout auto data, size_t len){
        if constexpr(sizeof(data) != 1){
            return i2c_drv_.write_repeat(DATA_TOKEN, std::span(data, len), LSB);
        }else {
            return i2c_drv_.write_repeat(DATA_TOKEN, data, len);
        }
    }

    void write_u8(const uint8_t data, size_t len) {
        write_burst<uint8_t>(data, len);
    }

    void write_u8(const uint8_t * data, size_t len) {
        write_burst<uint8_t>(data, len);
    }
};

};