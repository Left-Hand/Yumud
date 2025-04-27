#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "core/clock/clock.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "types/rgb.h"


namespace ymd::drivers{
namespace details{
enum class DisplayerError_Kind:uint8_t{
    CantSetup,
    WrongPc,
    MagnetLost,
    MagnetWeak,
    MagnetOverflow
};
}
DEF_ERROR_SUMWITH_BUSERROR(DisplayerError, details::DisplayerError_Kind)
}


namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::DisplayerError, hal::BusError> {
        static Result<T, drivers::DisplayerError> convert(const hal::BusError berr){
            
            if(berr.is_ok()) return Ok();
            return Err(berr); 
        }
    };
}


namespace ymd::drivers{

class DisplayerPhyIntf{
public:
    using Error = DisplayerError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    [[nodiscard]] virtual IResult<> init() = 0;

    [[nodiscard]] virtual IResult<> write_command(const uint32_t cmd) = 0;
    [[nodiscard]] virtual IResult<> write_data(const uint32_t data) = 0;

    [[nodiscard]] virtual IResult<> write_u8(const uint8_t data, size_t len) = 0;
    [[nodiscard]] virtual IResult<> write_u8(const uint8_t * data, size_t len) = 0;
};

class DisplayerPhySpi final:public DisplayerPhyIntf{
protected:
    hal::SpiDrv spi_drv_;
    hal::GpioIntf & dc_gpio_;
    hal::GpioIntf & res_gpio_;
    hal::GpioIntf & blk_gpio_;

    static constexpr auto COMMAND_LEVEL = LOW;
    static constexpr auto DATA_LEVEL = HIGH;
public:

    DisplayerPhySpi(
            hal::SpiDrv && spi_drv, 
            hal::GpioIntf & dc_gpio, 
            hal::GpioIntf & res_gpio = hal::NullGpio,
            hal::GpioIntf & blk_gpio = hal::NullGpio
        ) :spi_drv_(std::move(spi_drv)), dc_gpio_(dc_gpio), res_gpio_(res_gpio), blk_gpio_(blk_gpio){}

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

    [[nodiscard]] IResult<> init(){
        dc_gpio_.outpp();
        res_gpio_.outpp(HIGH);
        blk_gpio_.outpp(HIGH);

        return reset();
    }

    [[nodiscard]] IResult<> reset(){
        delay(10);
        res_gpio_.clr();
        delay(10);
        res_gpio_.set();
        return Ok();
    }

    [[nodiscard]] IResult<> set_back_light(const uint8_t brightness){
        return Ok();
    }

    [[nodiscard]] IResult<> write_command(const uint32_t cmd){
        dc_gpio_ = COMMAND_LEVEL;
        return IResult<>(spi_drv_.write_single<uint8_t>(cmd));
    }

    [[nodiscard]] IResult<> write_data(const uint32_t data){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_single<uint8_t>(data));
    }

    [[nodiscard]] IResult<> write_data16(const uint32_t data){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_single<uint16_t>(data));
    }

    [[nodiscard]] IResult<> write_single(const auto & data){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_single(data));
    }

    template<typename U>
    [[nodiscard]] IResult<> write_burst(const auto * data, size_t len){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_burst<U>(data, len));
    }

    template<typename U>
    [[nodiscard]] IResult<> write_burst(const auto & data, size_t len){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_burst<U>(data, len));
    }


    [[nodiscard]] IResult<> write_u8(const uint8_t data, size_t len){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_burst<uint8_t>(data, len));
    }

    [[nodiscard]] IResult<> write_u8(const uint8_t * data, size_t len){
        dc_gpio_ = DATA_LEVEL;
        return IResult<>(spi_drv_.write_burst<uint8_t>(data, len));
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

    [[nodiscard]] IResult<> init(){
        return Ok();
    }

    [[nodiscard]] IResult<> write_command(const uint32_t cmd){
        return IResult<>(i2c_drv_.write_reg<uint8_t>(CMD_TOKEN, uint8_t(cmd)));
    }

    [[nodiscard]] IResult<> write_data(const uint32_t data){
        return IResult<>(i2c_drv_.write_reg<uint8_t>(DATA_TOKEN, uint8_t(data)));
    }

    [[nodiscard]] IResult<> write_burst(const is_stdlayout auto * pdata, size_t len){
        if constexpr(sizeof(*pdata) != 1){
            return IResult<>(i2c_drv_.write_burst(DATA_TOKEN, std::span(pdata, len), LSB));
        }else {
            return IResult<>(i2c_drv_.write_burst(DATA_TOKEN, std::span(pdata, len)));
        }
    }

    [[nodiscard]] IResult<> write_burst(const is_stdlayout auto data, size_t len){
        if constexpr(sizeof(data) != 1){
            return IResult<>(i2c_drv_.write_repeat(DATA_TOKEN, std::span(data, len), LSB));
        }else {
            return IResult<>(i2c_drv_.write_repeat(DATA_TOKEN, data, len));
        }
    }

    [[nodiscard]] IResult<> write_u8(const uint8_t data, size_t len) {
        return write_burst<uint8_t>(data, len);
    }

    [[nodiscard]] IResult<> write_u8(const uint8_t * data, size_t len) {
        return write_burst<uint8_t>(data, len);
    }
};

};