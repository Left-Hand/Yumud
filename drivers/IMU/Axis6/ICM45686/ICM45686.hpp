#pragma once

#include "icm45686_prelude.hpp"


// https://github.com/NOKOLat/STM32_ICM45686/blob/master/ICM45686.h

namespace ymd::drivers{


class ICM45686 final:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    private ICM45686_Regs{
public:
    using Error = ICM45686_Prelude::Error;

    explicit ICM45686(Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        phy_(i2c, i2c_addr){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> set_mode(Mode mode);

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<Vec3<q24>> read_acc();
    [[nodiscard]] IResult<Vec3<q24>> read_gyr();
private:
    InvensenseSensor_Phy phy_;
    
    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data){
        return phy_.write_reg(addr, data);
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const T & reg){return write_reg(reg.address, reg);}

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data){
        return phy_.read_reg(addr, data);
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){return read_reg(reg.address, reg);}

};

}
