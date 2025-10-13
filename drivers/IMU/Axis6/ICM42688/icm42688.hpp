#pragma once

#include "details/ICM42688_Prelude.hpp"
#include "core/utils/enum_array.hpp"
#include "core/utils/angle.hpp"

namespace ymd::drivers{

class ICM42688:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    public ICM42688_Regs
{
public:
    explicit ICM42688(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> i2c_addr):
        phy_(i2c, i2c_addr){;}
    explicit ICM42688(Some<hal::Spi *> spi, const hal::SpiSlaveRank idx):
        phy_(spi, idx){;}
    explicit ICM42688(hal::SpiDrv && spi_drv):
        phy_(std::move(spi_drv)){;}

    [[nodiscard]] IResult<> init(const Config & cfg);
    
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<Vec3<q24>> read_acc();
    [[nodiscard]] IResult<Vec3<q24>> read_gyr();

    [[nodiscard]] IResult<> set_gyr_odr(const GyrOdr odr);
    [[nodiscard]] IResult<> set_gyr_fs(const GyrFs fs);
    [[nodiscard]] IResult<> set_acc_odr(const AccOdr odr);
    [[nodiscard]] IResult<> set_acc_fs(const AccFs fs);
    
private:
    InvensenseSensor_Phy phy_;
    Option<Bank> last_bank_ = None;

    q24 acc_scale_ = 0;
    q24 gyr_scale_ = 0;


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

    [[nodiscard]] static constexpr q24 calc_gyr_scale(const GyrFs fs){
        switch(fs){
            case GyrFs::_2000deg  :      return DEG2RAD<q24>(q24(2 * 2000   ));
            case GyrFs::_1000deg  :      return DEG2RAD<q24>(q24(2 * 1000   ));
            case GyrFs::_500deg   :      return DEG2RAD<q24>(q24(2 * 500    ));
            case GyrFs::_250deg   :      return DEG2RAD<q24>(q24(2 * 250    ));
            case GyrFs::_125deg   :      return DEG2RAD<q24>(q24(2 * 125    ));
            case GyrFs::_62_5deg  :      return DEG2RAD<q24>(q24(2 * 62.5   ));
            case GyrFs::_31_25deg :      return DEG2RAD<q24>(q24(2 * 31.25  ));
            case GyrFs::_15_625deg:      return DEG2RAD<q24>(q24(2 * 15.625 ));
        }
        __builtin_unreachable();
    }

    [[nodiscard]] static constexpr q24 calc_acc_scale(const AccFs fs){
        switch(fs){
            case AccFs::_16G    :       return GRAVITY_ACC<q24> * 32;
            case AccFs::_8G     :       return GRAVITY_ACC<q24> * 16;
            case AccFs::_4G     :       return GRAVITY_ACC<q24> * 8;
            case AccFs::_2G     :       return GRAVITY_ACC<q24> * 4;
        }
        __builtin_unreachable();
    }

};

}
