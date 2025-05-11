#pragma once

#include "details/ICM42688_collections.hpp"

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

    Option<Vector3_t<real_t>> get_acc();
    Option<Vector3_t<real_t>> get_gyr();
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
        constexpr  q24 LSB_GYR_2000_R	= q24(0.0010652644);
        constexpr  q24 LSB_GYR_1000_R	= q24(0.00053263222);
        constexpr  q24  LSB_GYR_500_R	= q24(0.00026631611);
        constexpr  q24  LSB_GYR_250_R	= q24(0.00013315805);
        constexpr  q24 LSB_GYR_125_R  	= q24(0.000066579027);	
        constexpr  q24 LSB_GYR_62_5_R  	= q24(0.000066579027/2);	
        constexpr  q24 LSB_GYR_31_25_R  	= q24(0.000066579027/4);	
        constexpr  q24 LSB_GYR_15_625_R  	= q24(0.000066579027/8);	

        switch(fs){
            case GyrFs::_2000DPS:   return LSB_GYR_2000_R;
            case GyrFs::_1000DPS:   return LSB_GYR_1000_R;
            case GyrFs::_500DPS:    return LSB_GYR_500_R;
            case GyrFs::_250DPS:    return LSB_GYR_250_R;
            case GyrFs::_125DPS:    return LSB_GYR_125_R;
            case GyrFs::_62_5DPS:   return LSB_GYR_62_5_R;
            case GyrFs::_31_25DPS:  return LSB_GYR_31_25_R;
            case GyrFs::_15_625DPS: return LSB_GYR_15_625_R;
            default: __builtin_unreachable();
            
        }
    }

    static constexpr q24 calc_acc_lsb(const AccFs fs){

        constexpr q24 LSB_ACC_16G = q24( 0.0047856934);
        constexpr q24  LSB_ACC_8G = q24( 0.0023928467);
        constexpr q24  LSB_ACC_4G = q24( 0.0011964233);
        constexpr q24  LSB_ACC_2G = q24(0.00059821167);
        switch(fs){
            case AccFs::_16G: return LSB_ACC_16G;
            case AccFs::_8G: return LSB_ACC_8G;
            case AccFs::_4G: return LSB_ACC_4G;
            case AccFs::_2G: return LSB_ACC_2G;
            default: __builtin_unreachable();
        }
    }

    IResult<> set_gyr_odr(const GyrOdr odr);
    IResult<> set_gyr_fs(const GyrFs fs);
    IResult<> set_acc_odr(const AccOdr odr);
    IResult<> set_acc_fs(const AccFs fs);
    
    q24 lsb_acc_;
    q24 lsb_gyr_;
};

}
