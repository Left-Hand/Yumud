#pragma once

#include "details/ICM42688_Prelude.hpp"
#include "core/utils/enum/enum_array.hpp"
#include "primitive/arithmetic/angular.hpp"

namespace ymd::drivers::icm42688{

class ICM42688 final{
public:
    using Self = ICM42688;

    explicit ICM42688(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR
    ):
        transport_(i2c, i2c_addr){;}
    explicit ICM42688(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(spi, rank){;}
    explicit ICM42688(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}

    [[nodiscard]] IResult<> init(const Config & cfg);
    
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<Vec3<iq24>> read_acc();
    [[nodiscard]] IResult<Vec3<iq24>> read_gyr();

    [[nodiscard]] IResult<> set_gyr_odr(const GyrOdr odr);
    [[nodiscard]] IResult<> set_gyr_fs(const GyrFs fs);
    [[nodiscard]] IResult<> set_acc_odr(const AccOdr odr);
    [[nodiscard]] IResult<> set_acc_fs(const AccFs fs);
    
private:
    Regset regs_ = {};
    InvensenseImu_Transport transport_;
    Option<Bank> last_bank_ = None;

    iq16 acc_scale_ = 0;
    iq16 gyr_scale_ = 0;


    [[nodiscard]] IResult<> switch_bank(const Bank bank){
        static constexpr uint8_t SWITCH_BANK_COMMAND = 0x76; 
        if(last_bank_.is_some() and (last_bank_.unwrap() == bank))
            return Ok();
        if(const auto res = transport_.write_reg(SWITCH_BANK_COMMAND, static_cast<uint8_t>(bank));
            res.is_err()) return Err(res.unwrap_err());
        last_bank_ = Some(bank);
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        if(const auto res = switch_bank(reg.bank);
            res.is_err()) return res;
        if(const auto res = transport_.write_reg(T::ADDRESS, reg.to_bits());
            res.is_err()) return res;
        reg.apply();
        return Ok();
    }


    [[nodiscard]] IResult<> write_reg(const uint8_t reg_addr, const uint8_t reg_val){
        if(const auto res = transport_.write_reg(reg_addr, reg_val);
            res.is_err()) return res;
        return Ok();
    }

    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        if(const auto res = switch_bank(reg.bank);
            res.is_err()) return res;
        return transport_.read_reg(T::ADDRESS, reg.as_bits_mut());
    };

    [[nodiscard]] static constexpr iq16 calc_gyr_scale(const GyrFs fs){
        switch(fs){
            case GyrFs::_2000deg  :      return iq16(2 * 2000   * PI / 180);
            case GyrFs::_1000deg  :      return iq16(2 * 1000   * PI / 180);
            case GyrFs::_500deg   :      return iq16(2 * 500    * PI / 180);
            case GyrFs::_250deg   :      return iq16(2 * 250    * PI / 180);
            case GyrFs::_125deg   :      return iq16(2 * 125    * PI / 180);
            case GyrFs::_62_5deg  :      return iq16(2 * 62.5   * PI / 180);
            case GyrFs::_31_25deg :      return iq16(2 * 31.25  * PI / 180);
            case GyrFs::_15_625deg:      return iq16(2 * 15.625 * PI / 180);
        }
        __builtin_unreachable();
    }

    [[nodiscard]] static constexpr iq16 calc_acc_scale(const AccFs fs){
        switch(fs){
            case AccFs::_16G    :       return iq16(9.80665 * 32);
            case AccFs::_8G     :       return iq16(9.80665 * 16);
            case AccFs::_4G     :       return iq16(9.80665 * 8);
            case AccFs::_2G     :       return iq16(9.80665 * 4);
        }
        __builtin_unreachable();
    }

};

}
