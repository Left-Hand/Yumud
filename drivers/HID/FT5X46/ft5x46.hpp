#pragma once
#include "ft5x46_prelude.hpp"

//参考：
    // SPDX-License-Identifier: Apache-2.0
    // https://github.com/ArmSoM/rk3506-rkr4.2-sdk/blob/7066d7b709d07fe21dc1808017ba11cc9987cfcf/rtos/bsp/nuvoton/libraries/nu_packages/TPC/ft5446.c

namespace ymd::drivers::ft5x46{


class FT5X46:public FT5X46_Prelude{
public:
    explicit FT5X46(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}
    explicit FT5X46(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}
    explicit FT5X46(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv{i2c, i2c_addr}){;}

    IResult<> init();

private:
    hal::I2cDrv i2c_drv_;
    FT5X46_Regs regs_ = {};

    using Points = std::array<Option<Point>, NUM_MAX_TOUCH_POINTS>;
    Points points_ = {None, None, None, None, None};



    
    IResult<> write_reg(const uint8_t reg_addr, const uint8_t reg_val);
    
    IResult<> read_reg(const uint8_t reg_addr, uint8_t & reg_val);


    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        return write_reg(T::REG_ADDR, reg.to_bits());
    }


    template<typename T>
    IResult<> read_reg(T & reg){
        const auto res = read_reg(T::REG_ADDR);
        if(res.is_err()) return Err(res.unwrap_err());
        reg.as_bits_mut() = res.unwrap();
        return Ok();
    }

    IResult<> read_burst(const uint8_t reg_addr, std::span<uint8_t> buffer){
        if(const auto res = i2c_drv_.read_burst(reg_addr, buffer);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    IResult<uint16_t> read_u12(const uint8_t reg_addr){
        uint16_t ret;
        if(const auto res = read_burst_u12(reg_addr, std::span(&ret, 1));
            res.is_err()) return Err(res.unwrap_err());
        return Ok(ret);
    }

    IResult<> read_burst_u12(const uint8_t reg_addr, std::span<uint16_t> buffer){
        if(const auto res = i2c_drv_.read_burst(reg_addr, buffer, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());

        for(auto & item : buffer){
            item = item & 0x0fff;
        }

        return Ok();
    }
};

}