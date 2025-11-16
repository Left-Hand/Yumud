#pragma once

#include "ina219_prelude.hpp"

namespace ymd::drivers{

class INA219 final:
    public INA219_Regs{
public:

    explicit INA219(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit INA219(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit INA219(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};


    IResult<> update();

    IResult<> validate();

private:
    hal::I2cDrv i2c_drv_;
    
    real_t current_lsb_ma_ = real_t(0.2);

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(reg.address, reg.as_bits());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }
    
    [[nodiscard]] IResult<> write_reg(const RegAddr addr, const uint16_t data);

    [[nodiscard]] IResult<> read_reg(const RegAddr addr, uint16_t & data);
    
    [[nodiscard]] IResult<> read_reg(const RegAddr addr, int16_t & data);

    [[nodiscard]] IResult<> read_burst(const RegAddr addr, std::span<uint16_t> pbuf);
};
}
