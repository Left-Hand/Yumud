#pragma once

#include "icm45686_prelude.hpp"

// 参考来源：

// 无许可证
// https://github.com/NOKOLat/STM32_ICM45686/blob/master/ICM45686.h

//  * 注意：本实现为完全原创，未使用上述项目的任何代码。
//  * 参考仅用于理解问题领域，未复制任何具体实现。


namespace ymd::drivers{


class ICM45686 final:
    private ICM45686_Regs{
public:
    using Error = ICM45686_Prelude::Error;

    explicit ICM45686(Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        transport_(i2c, i2c_addr){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> set_mode(Mode mode);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<math::Vec3<iq24>> read_acc();
    [[nodiscard]] IResult<math::Vec3<iq24>> read_gyr();
private:
    InvensenseImu_Transport transport_;
    
    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t reg_val){
        return transport_.write_reg(addr, reg_val);
    }


    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){return read_reg(T::ADDRESS, reg);}

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::ADDRESS, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }


    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & reg_val){
        return transport_.read_reg(addr, reg_val);
    }

};

}
