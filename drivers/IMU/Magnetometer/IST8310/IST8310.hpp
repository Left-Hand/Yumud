#pragma once

#include "ist8310_prelude.hpp"

namespace ymd::drivers{

class IST8310 final:
    public IST8310_Prelude{
public:

    explicit IST8310(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit IST8310(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit IST8310(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){;}

    IResult<> init();
    IResult<> update();

    IResult<> validate();

    IResult<> reset();

    IResult<> enable_continous(const Enable en);

    IResult<> set_x_average_times(const AverageTimes times);
    IResult<> set_y_average_times(const AverageTimes times);

    IResult<iq16> get_temperature();

    IResult<bool> is_data_ready();
    IResult<> enable_interrupt(const Enable en);
    IResult<> set_interrupt_level(const BoolLevel lv);
    IResult<bool> get_interrupt_status();

    IResult<> enable_sleep(const Enable en);

    IResult<math::Vec3<iq24>> read_mag();

private:
    hal::I2cDrv i2c_drv_;
    IST8310_Regset regs_ = {};

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = i2c_drv_.write_reg(uint8_t(T::REG_ADDR), reg.to_bits(), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        if(const auto res = i2c_drv_.read_reg(uint8_t(T::REG_ADDR), reg.as_bits_mut(), std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<> read_burst(const RegAddr addr, std::span<int16_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(std::bit_cast<uint8_t>(addr), pbuf, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

};

}