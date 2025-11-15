#pragma once

#include "ist8310_prelude.hpp"

namespace ymd::drivers{

class IST8310 final:
    public MagnetometerIntf,
    public IST8310_Prelude{
public:

    explicit IST8310(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    explicit IST8310(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit IST8310(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> enable_contious(const Enable en);

    [[nodiscard]] IResult<> set_x_average_times(const AverageTimes times);
    [[nodiscard]] IResult<> set_y_average_times(const AverageTimes times);

    [[nodiscard]] IResult<iq16> get_temperature();

    [[nodiscard]] IResult<bool> is_data_ready();
    [[nodiscard]] IResult<> enable_interrupt(const Enable en);
    [[nodiscard]] IResult<> set_interrupt_level(const BoolLevel lv);
    [[nodiscard]] IResult<bool> get_interrupt_status();

    [[nodiscard]] IResult<> enable_sleep(const Enable en);

    [[nodiscard]] IResult<Vec3<iq24>> read_mag() override;

private:
    hal::I2cDrv i2c_drv_;
    IST8310_Regset regs_ = {};

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(T::ADDRESS), reg.as_bits(), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(T::ADDRESS), reg.as_mut_bits(), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const RegAddr addr, int16_t * pbuf, size_t len){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr), std::span(pbuf, len), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

};

}