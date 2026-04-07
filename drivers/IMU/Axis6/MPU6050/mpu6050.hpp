#pragma once

//这个驱动已经完成了基础使用

#include "mpu6050_prelude.hpp"

namespace ymd::drivers{
class MPU6050 final:
    public MPU6050_Prelude{
public:

    explicit MPU6050(const hal::I2cDrv & i2c_drv):
        MPU6050(i2c_drv, Package::MPU6050){;}

    explicit MPU6050(hal::I2cDrv && i2c_drv):
        MPU6050(std::move(i2c_drv), Package::MPU6050){;}

    explicit MPU6050(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        MPU6050(hal::I2cDrv(i2c, i2c_addr), Package::MPU6050){;}

    MPU6050(const MPU6050 & other) = delete;
    MPU6050(MPU6050 && other) = delete;

    IResult<> validate();

    IResult<> init(const Config & cfg);
    
    IResult<> update();

    IResult<math::Vec3<iq24>> read_acc();
    IResult<math::Vec3<iq24>> read_gyr();
    IResult<iq16> read_temp();

    IResult<> set_acc_fs(const AccFs fs);
    IResult<> set_gyr_fs(const GyrFs fs);

    IResult<> reset();

    void set_package(const Package package){
        package_ = package;
    }

    [[nodiscard]] Result<Package, Error> get_package();

    IResult<> enable_direct_mode(const Enable en);
private:

    using Phy = InvensenseImu_Transport;
    Phy transport_;
    MPU6050_Regset regs_ = {};
    iq16 acc_scaler_ = 0;
    iq16 gyr_scaler_ = 0;

    bool is_data_valid_ = false;
    Package package_ = Package::MPU6050;

    MPU6050(const hal::I2cDrv i2c_drv, const Package package);

    IResult<> write_reg(const uint8_t addr, const uint8_t data){
        return transport_.write_reg(addr, data);
    }

    template<typename T>
    IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(T::REG_ADDR, reg.to_bits());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    IResult<> read_reg(const uint8_t reg_addr, uint8_t & reg_val){
        return transport_.read_reg(reg_addr, reg_val);
    }

    IResult<> read_bulk(const uint8_t reg_addr, std::span<int16_t> pbuf){
        return transport_.read_bulk(reg_addr, pbuf);
    }

    template<typename T>
    IResult<> read_reg(T & reg){
        return read_reg(T::REG_ADDR, reg.as_bits_mut());
    }



};

};