#pragma once

#include "lis2dw12_prelude.hpp"


namespace ymd::drivers{


class LIS2DW12:public LIS2DW12_Prelude{
public:
    explicit LIS2DW12(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):
        transport_(hal::I2cDrv{i2c, i2c_addr}){;}
    explicit LIS2DW12(const hal::I2cDrv & i2c_drv):
        transport_(i2c_drv){;}
    explicit LIS2DW12(hal::I2cDrv && i2c_drv):
        transport_(std::move(i2c_drv)){;}
    explicit LIS2DW12(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit LIS2DW12(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit LIS2DW12(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(hal::SpiDrv{spi, rank}){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_acc_odr(const AccOdr odr);
    [[nodiscard]] IResult<> set_acc_fs(const AccFs range);
    
    [[nodiscard]] IResult<> set_pmu_mode(const PmuType pum, const PmuMode mode);
    [[nodiscard]] IResult<Vec3<iq24>> read_acc();

private:
    using Phy = StmicroImu_Transport;
    Phy transport_;

    iq16 acc_scale = 0;
    iq16 gyr_scale = 0;

    [[nodiscard]] IResult<> write_reg(uint8_t reg_addr, uint8_t reg_val){
        if(const auto res = transport_.write_reg(reg_addr, reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(uint8_t reg_addr, uint8_t & reg_val){
        if(const auto res = transport_.read_reg(reg_addr, reg_val);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

    // [[nodiscard]] IResult<> read_burst(uint8_t reg_addr, std::span<uint8_t> pbuf){
    //     if(const auto res = transport_.read_burst(reg_addr, pbuf, std::endian::little);
    //         res.is_err()) return Err(res.unwrap_err());
    //     return Ok();
    // }

    [[nodiscard]] static constexpr iq24 calc_gyr_scaler(const GyrFs fs){
        switch(fs){
            case GyrFs::_2000deg  :      return iq24(DEG2RAD_RATIO) * 2 * 2000;
            case GyrFs::_1000deg  :      return iq24(DEG2RAD_RATIO) * 2 * 1000;
            case GyrFs::_500deg   :      return iq24(DEG2RAD_RATIO) * 2 * 500;
            case GyrFs::_250deg   :      return iq24(DEG2RAD_RATIO) * 2 * 250;
            case GyrFs::_125deg   :      return iq24(DEG2RAD_RATIO) * 2 * 125;
        }
        __builtin_unreachable();
    }

    [[nodiscard]] static constexpr iq24 calc_acc_scaler(const AccFs fs){
        switch(fs){
            case AccFs::_16G    :       return GRAVITY_ACC<iq24> * 32;
            case AccFs::_8G     :       return GRAVITY_ACC<iq24> * 16;
            case AccFs::_4G     :       return GRAVITY_ACC<iq24> * 8;
            case AccFs::_2G     :       return GRAVITY_ACC<iq24> * 4;
        }
        __builtin_unreachable();
    }

};

}