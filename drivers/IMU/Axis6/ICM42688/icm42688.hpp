#pragma once

#include "details/ICM42688_collections.hpp"
#include "core/utils/EnumArray.hpp"

namespace ymd::drivers{

class ICM42688:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    public ICM42688_Regs
{
public:
    ICM42688(hal::I2c & i2c, const hal::I2cSlaveAddr<7> i2c_addr = DEFAULT_I2C_ADDR):phy_(i2c, i2c_addr){;}
    ICM42688(hal::Spi & spi, const hal::SpiSlaveIndex idx):phy_(spi, idx){;}
    ICM42688(hal::SpiDrv && spi_drv):phy_(std::move(spi_drv)){;}

    IResult<> init();
    
    IResult<> update();

    IResult<> validate();

    IResult<> reset();

    IResult<Vector3_t<q24>> read_acc();
    IResult<Vector3_t<q24>> read_gyr();
private:
    InvensenseSensor_Phy phy_;
    Option<Bank> last_bank_ = None;

    IResult<> switch_bank(const Bank bank){
        if(last_bank_.is_none() or (last_bank_.unwrap() != bank)){
            const auto res = phy_.write_reg(0x76, static_cast<uint8_t>(bank));
            last_bank_ = Some(bank);
            return res;
        }
        return Ok();
    }

    IResult<> write_reg(const auto & reg){
        if(const auto res = switch_bank(reg.bank);
            res.is_err()) return res;
        return phy_.write_reg(reg.address, reg.as_val());
    }

    IResult<> read_reg(auto & reg){
        if(const auto res = switch_bank(reg.bank);
            res.is_err()) return res;
        return phy_.read_reg(reg.address, reg.as_ref());
    };

    static constexpr q24 calc_gyr_lsb(const GyrFs fs){
        /*Turn Into Radian*/
        constexpr EnumArray<GyrFs, q24> map = {
            q24(0.0010652644),
            q24(0.00053263222),
            q24(0.00026631611),
            q24(0.00013315805),
            q24(0.000066579027),
            q24(0.000066579027/2),
            q24(0.000066579027/4),
            q24(0.000066579027/8)
        };

        return map[fs];
    }

    static constexpr q24 calc_acc_lsb(const AccFs fs){
        constexpr EnumArray<AccFs, q24> map = {
            q24( 0.0047856934),
            q24( 0.0023928467),
            q24( 0.0011964233),
            q24(0.00059821167)
        };

        return map[fs];
    }

    IResult<> set_gyr_odr(const GyrOdr odr);
    IResult<> set_gyr_fs(const GyrFs fs);
    IResult<> set_acc_odr(const AccOdr odr);
    IResult<> set_acc_fs(const AccFs fs);
    
    q24 lsb_acc_;
    q24 lsb_gyr_;
};

}
