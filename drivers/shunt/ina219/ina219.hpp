#pragma once

#include "ina219_prelude.hpp"

namespace ymd::drivers{

class INA219 final:
    public INA219_Prelude{
public:

    explicit INA219(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit INA219(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit INA219(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv(i2c, i2c_addr)){};

    INA219(const INA219 &) = delete;
    INA219(INA219 &&) = delete;
    ~INA219() = default;


    IResult<> update();

    IResult<> validate();

private:
    hal::I2cDrv i2c_drv_;
    INA219_Regs regs_ = {}; 
    
    iq16 current_lsb_ma_ = iq16(0.2);

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(reg.address, reg.to_bits());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }
    
    IResult<> write_reg(const RegAddr addr, const uint16_t data);

    IResult<> read_reg(const RegAddr addr, uint16_t & data);
    
    IResult<> read_reg(const RegAddr addr, int16_t & data);

    IResult<> read_bulk(const RegAddr addr, std::span<uint16_t> pbuf);
};
}
