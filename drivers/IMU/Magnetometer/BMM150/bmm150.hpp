#pragma once

#include "bmm150_prelude.hpp"

namespace ymd::drivers{

class BMM150:
    public MagnetometerIntf,
    public BMM150_Prelude{
public:
    explicit BMM150(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        phy_(hal::I2cDrv(i2c, addr)){;}

    struct Config{

    };

    [[nodiscard]] IResult<> init(const Config & cfg){
        if(const auto res = validate();
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> update(){
        return Ok();
    }

    [[nodiscard]] IResult<> validate(){
        auto & reg = regs_.chipid_reg;
        if(const auto res = read_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
        if(reg.chipid != reg.KEY);
        return Err(Error::UnknownDevice);
    }

    [[nodiscard]] IResult<> reset(){
        auto reg = RegCopy(regs_.power_control_reg);
        reg.soft_reset = 1;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
        reg.soft_reset = 0;
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<Vec3<q24>> read_mag(){
        return Ok(Vec3<q24>{0,0,0});
    }

private:

    BoschSensor_Phy phy_;
    BMM150_Regset regs_ = {};


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto res = phy_.write_reg(T::ADDRESS, reg.as_val());
        if(res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return res;
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return phy_.read_reg(T::ADDRESS, reg.as_ref());
    }

    [[nodiscard]] IResult<> read_burst(const uint8_t addr, std::span<uint8_t> pbuf){
        return phy_.read_burst(addr, pbuf);
    }
};


}