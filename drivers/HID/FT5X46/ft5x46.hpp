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
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        i2c_drv_(hal::I2cDrv{i2c, addr}){;}

    [[nodiscard]] IResult<> init();

private:
    hal::I2cDrv i2c_drv_;
    FT5X46_Regs regs_;

    using Points = std::array<Option<Point>, NUM_MAX_TOUCH_POINTS>;
    Points points_;

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data);

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        return write_reg(T::ADDRESS, reg.to_bits());
    }

    [[nodiscard]] IResult<uint8_t> read_reg(const uint8_t addr);

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        const auto res = read_reg(T::ADDRESS);
        if(res.is_err()) return Err(res.unwrap_err());
        reg.as_bits_mut() = res.unwrap();
        return Ok();
    }

    [[nodiscard]] IResult<> read_burst(const uint8_t addr, std::span<uint8_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(addr, pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<uint16_t> read_u12(const uint8_t addr){
        uint16_t ret;
        if(const auto res = read_burst_u12(addr, std::span(&ret, 1));
            res.is_err()) return Err(res.unwrap_err());
        return Ok(ret);
    }

    [[nodiscard]] IResult<> read_burst_u12(const uint8_t addr, std::span<uint16_t> pbuf){
        if(const auto res = i2c_drv_.read_burst(addr, pbuf, std::endian::big);
            res.is_err()) return Err(res.unwrap_err());
        for(auto & item : pbuf){
            item = item & 0x0fff;
        }
        return Ok();
    }
};

}