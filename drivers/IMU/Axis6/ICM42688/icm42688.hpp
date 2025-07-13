#pragma once

#include "details/ICM42688_Prelude.hpp"
#include "core/utils/enum_array.hpp"

namespace ymd::drivers{

class ICM42688:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    public ICM42688_Regs
{
public:
    ICM42688(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):phy_(i2c, i2c_addr){;}
    ICM42688(Some<hal::Spi *> spi, const hal::SpiSlaveIndex idx):phy_(spi, idx){;}
    ICM42688(hal::SpiDrv && spi_drv):phy_(std::move(spi_drv)){;}

    [[nodiscard]] IResult<> init(const Config & cfg);
    
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<Vector3<q24>> read_acc();
    [[nodiscard]] IResult<Vector3<q24>> read_gyr();
private:
    InvensenseSensor_Phy phy_;
    Option<Bank> last_bank_ = None;

    [[nodiscard]] IResult<> switch_bank(const Bank bank){
        if(last_bank_.is_none() or (last_bank_.unwrap() != bank)){
            const auto res = phy_.write_reg(0x76, static_cast<uint8_t>(bank));
            last_bank_ = Some(bank);
            return res;
        }
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = switch_bank(reg.bank);
            res.is_err()) return res;
        if(const auto res = phy_.write_reg(reg.address, reg.as_val());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }

    [[nodiscard]] IResult<> read_reg(auto & reg){
        if(const auto res = switch_bank(reg.bank);
            res.is_err()) return res;
        return phy_.read_reg(reg.address, reg.as_ref());
    };

    static constexpr q24 calc_gyr_scale(const GyrFs fs){
        switch(fs){
            case GyrFs::_2000deg  :      return 2000_deg;
            case GyrFs::_1000deg  :      return 1000_deg;
            case GyrFs::_500deg   :      return 500_deg;
            case GyrFs::_250deg   :      return 250_deg;
            case GyrFs::_125deg   :      return 125_deg;
            case GyrFs::_62_5deg  :      return 62.5_deg;
            case GyrFs::_31_25deg :      return 31.25_deg;
            case GyrFs::_15_625deg:      return 15.625_deg;
            default: __builtin_unreachable();
        }
    }

    static constexpr q24 calc_acc_scale(const AccFs fs){
        switch(fs){
            case AccFs::_16G    :       return GRAVITY_ACC<q24> * 16;
            case AccFs::_8G     :       return GRAVITY_ACC<q24> * 8;
            case AccFs::_4G     :       return GRAVITY_ACC<q24> * 4;
            case AccFs::_2G     :       return GRAVITY_ACC<q24> * 2;
            default: __builtin_unreachable();
        }
    }

    [[nodiscard]] IResult<> set_gyr_odr(const GyrOdr odr);
    [[nodiscard]] IResult<> set_gyr_fs(const GyrFs fs);
    [[nodiscard]] IResult<> set_acc_odr(const AccOdr odr);
    [[nodiscard]] IResult<> set_acc_fs(const AccFs fs);
    
    q24 acc_scale_;
    q24 gyr_scale_;
};

}
