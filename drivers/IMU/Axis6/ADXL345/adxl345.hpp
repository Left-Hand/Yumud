#pragma once

#include "adxl345_prelude.hpp"

namespace ymd::drivers{

class ADXL345:
    public AccelerometerIntf, 
    public ADXL345_Prelude
{

public:
    explicit ADXL345(const hal::I2cDrv & i2c_drv): 
        phy_(i2c_drv){;}

    explicit ADXL345(hal::I2cDrv && i2c_drv): 
        phy_(std::move(i2c_drv)){;}

    explicit ADXL345(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv(i2c, addr)){;}

    explicit ADXL345(const hal::SpiDrv & spi_drv): 
        phy_(spi_drv){;}

    explicit ADXL345(hal::SpiDrv && spi_drv): 
        phy_(std::move(spi_drv)){;}

    explicit ADXL345(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank): 
        phy_(hal::SpiDrv{spi, rank}){;}

    [[nodiscard]] IResult<Vec3<q24>> read_acc();
    
    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> self_test();
private:
    AnalogDeviceIMU_Phy phy_;
    ADXL345_Regset regs_ = {};
    q24 acc_scaler_ = 0;


    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = switch_bank(reg.bank);
            res.is_err()) return res;
        if(const auto res = phy_.write_reg(std::bit_cast<uint8_t>(reg.address), reg.as_val());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(auto & reg){
        return phy_.read_reg(std::bit_cast<uint8_t>(reg.address), reg.as_ref());
    };

    [[nodiscard]] IResult<> write_reg(const RegAddr reg_address, const uint8_t reg_data);

    [[nodiscard]] IResult<> read_reg(const RegAddr reg_address, uint8_t & reg_data);
};

};