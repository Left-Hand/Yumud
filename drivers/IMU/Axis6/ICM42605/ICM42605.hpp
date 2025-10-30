//这个驱动尚未完成
//这个驱动尚未测试

#pragma once

#include "icm42605_prelude.hpp"


namespace ymd::drivers{

class ICM42605 final:
    public ICM42605_Prelude,
    public AccelerometerIntf, 
    public GyroscopeIntf
{
public:
    explicit ICM42605(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        phy_(hal::I2cDrv(i2c, DEFAULT_I2C_ADDR)){;}

    [[nodiscard]] IResult<> init();
    
    [[nodiscard]] IResult<> validate();
    
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<Vec3<iq24>> read_acc();
    [[nodiscard]] IResult<Vec3<iq24>> read_gyr();

private:


    InvensenseSensor_Phy phy_;
    Option<Bank> last_bank_ = None;  

    iq16 acc_scaler_ = 0;
    iq16 gyr_scaler_ = 0;

    Vec3<int16_t> acc_data_ = Vec3<int16_t>::ZERO;
    Vec3<int16_t> gyr_data_ = Vec3<int16_t>::ZERO;

    [[nodiscard]] IResult<> switch_bank(const Bank bank){
        static constexpr uint8_t SWITCH_BANK_COMMAND = 0x76; 
        if(last_bank_.is_some() and (last_bank_.unwrap() == bank))
            return Ok();
        last_bank_ = Some(bank);
        return phy_.write_reg(SWITCH_BANK_COMMAND, static_cast<uint8_t>(bank));
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = switch_bank(reg.bank);
            res.is_err()) return res;
        if(const auto res = phy_.write_reg(T::ADDRESS, reg.as_val());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> write_reg(const uint8_t reg_addr, const uint8_t reg_val){
        if(const auto res = phy_.write_reg(reg_addr, reg_val);
            res.is_err()) return res;
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = switch_bank(reg.bank);
            res.is_err()) return res;
        return phy_.read_reg(T::ADDRESS, reg.as_ref());
    };

    [[nodiscard]] IResult<> read_reg(const uint8_t reg_addr, uint8_t & reg_val){
        if(const auto res = phy_.read_reg(reg_addr, reg_val);
            res.is_err()) return res;
        return Ok();
    }
    
};

}
