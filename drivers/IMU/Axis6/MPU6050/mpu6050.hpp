#pragma once

//这个驱动已经完成了基础使用

#include "mpu6050_prelude.hpp"

namespace ymd::drivers{
class MPU6050 final:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    public MPU6050_Regs{
public:

    explicit MPU6050(const hal::I2cDrv & i2c_drv):
        MPU6050(i2c_drv, Package::MPU6050){;}
    explicit MPU6050(hal::I2cDrv && i2c_drv):
        MPU6050(std::move(i2c_drv), Package::MPU6050){;}
    explicit MPU6050(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        MPU6050(hal::I2cDrv(i2c, addr), Package::MPU6050){;}

    MPU6050(const MPU6050 & other) = delete;
    MPU6050(MPU6050 && other) = delete;

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> init(const Config & cfg);
    
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<Vec3<q24>> read_acc();
    [[nodiscard]] IResult<Vec3<q24>> read_gyr();
    [[nodiscard]] IResult<q16> read_temp();

    [[nodiscard]] IResult<> set_acc_fs(const AccFs fs);
    [[nodiscard]] IResult<> set_gyr_fs(const GyrFs fs);

    [[nodiscard]] IResult<> reset();

    void set_package(const Package package){
        package_ = package;
    }

    [[nodiscard]] Result<Package, Error> get_package();

    [[nodiscard]] IResult<> enable_direct_mode(const Enable en);
private:

    using Phy = InvensenseSensor_Phy;
    Phy phy_;
    q16 acc_scaler_ = 0;
    q16 gyr_scaler_ = 0;

    bool is_data_valid_ = false;
    Package package_ = Package::MPU6050;

    MPU6050(const hal::I2cDrv i2c_drv, const Package package);

    [[nodiscard]] IResult<> write_reg(const uint8_t addr, const uint8_t data){
        return phy_.write_reg(addr, data);
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = write_reg(reg.address, reg.as_val());
            res.is_err()) return Err(res.unwrap_err());
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(const uint8_t addr, uint8_t & data){
        return phy_.read_reg(addr, data);
    }

    [[nodiscard]] IResult<> read_burst(const uint8_t addr, std::span<int16_t> pbuf){
        return phy_.read_burst(addr, pbuf);
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        return read_reg(reg.address, reg.as_ref());
    }

    static constexpr q16 calculate_acc_scale(const AccFs fs){
        constexpr double g = 9.806;
        switch(fs){
            case AccFs::_2G: return q16(g * 2);
            case AccFs::_4G: return q16(g * 4);
            case AccFs::_8G: return q16(g * 8);
            case AccFs::_16G: return q16(g * 16);
        }
        __builtin_unreachable();
    }

    static constexpr q16 calculate_gyr_scale(const GyrFs fs){
        switch(fs){
            case GyrFs::_250deg: return DEG2RAD<q16>(250);
            case GyrFs::_500deg: return DEG2RAD<q16>(500);
            case GyrFs::_1000deg: return DEG2RAD<q16>(1000);
            case GyrFs::_2000deg: return DEG2RAD<q16>(2000);
        }
        __builtin_unreachable();
    }

};

};