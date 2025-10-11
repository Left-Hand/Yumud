
//这个驱动还未完成
//这个驱动还未测试

//BMI088是博世科技推出的一款高精度IMU

// https://blog.csdn.net/weixin_44080304/article/details/125065724


#pragma once

#include "bmi088_prelude.hpp"

namespace ymd::drivers{
class BMI088_Acc final: 
    public AccelerometerIntf,
    public BMI088_Prelude{
public:
    explicit BMI088_Acc(const hal::I2cDrv & i2c_drv):
        phy_(i2c_drv){;}
    explicit BMI088_Acc(hal::I2cDrv && i2c_drv):
        phy_(std::move(i2c_drv)){;}
    explicit BMI088_Acc(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv{i2c, DEFAULT_I2C_ADDR}){;}

    explicit BMI088_Acc(const hal::SpiDrv & spi_drv):
        phy_(spi_drv){;}
    explicit BMI088_Acc(hal::SpiDrv && spi_drv):
        phy_(std::move(spi_drv)){;}
    explicit BMI088_Acc(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        phy_(hal::SpiDrv{spi, index}){;}


    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<Vec3<q24>> read_acc();
    [[nodiscard]] IResult<real_t> read_temp();

    [[nodiscard]] IResult<> set_acc_fs(const AccFs range);
    [[nodiscard]] IResult<> set_acc_bwp(const AccBwp bwp);
    [[nodiscard]] IResult<> set_acc_odr(const AccOdr odr);
private:
    BoschSensor_Phy phy_;
    real_t acc_scaler_ = 0;
    BMI088_AccRegs regs_ = {};


    [[nodiscard]] IResult<> verify_chip_id();

    #if 0
    // class InterruptChannel{
    // protected:
    //     using Error = BMI088_Acc::Error;
    // public:
    //     InterruptChannel(BMI088_Acc & bmi, _R8_IoCtrl & ctrl, const uint8_t address):
    //         bmi_(bmi), ctrl_(ctrl), address_(address){;}

    //     [[nodiscard]] IResult<> enable_output(const Enable en){
    //         auto reg = RegCopy(ctrl_);
    //         reg.int_out = en == EN;
    //         return bmi_.phy_.write_reg(reg);
    //     }
    // protected:
    //     BMI088_Acc & bmi_;
    //     _R8_IoCtrl & ctrl_;
    //     uint8_t address_;
    // };

    // friend InterruptChannel;
    // std::array<InterruptChannel, 2> interrupts = {
    //     InterruptChannel{*this, int1_ctrl_reg, int1_ctrl_reg.address},
    //     InterruptChannel{*this, int2_ctrl_reg, int2_ctrl_reg.address},
    // };
    #endif

    [[nodiscard]] static constexpr Option<real_t> 
    calculate_acc_scale(const AccFs range){
        constexpr double g = 9.806;
        switch(range){
            default:
                return None;
            case AccFs::_3G:
                return Some(real_t(g * 3));
            case AccFs::_6G:
                return Some(real_t(g * 6));
            case AccFs::_12G:
                return Some(real_t(g * 12));
            case AccFs::_24G:
                return Some(real_t(g * 24));
        }
    }
};



class BMI088_Gyr final:
    public GyroscopeIntf,
    public BMI088_Prelude{
public:
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;
public:
    explicit BMI088_Gyr(const hal::I2cDrv & i2c_drv):
        phy_(i2c_drv){;}
    explicit BMI088_Gyr(hal::I2cDrv && i2c_drv):
        phy_(std::move(i2c_drv)){;}
    explicit BMI088_Gyr(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv{i2c, DEFAULT_I2C_ADDR}){;}

    explicit BMI088_Gyr(const hal::SpiDrv & spi_drv):
        phy_(spi_drv){;}
    explicit BMI088_Gyr(hal::SpiDrv && spi_drv):
        phy_(std::move(spi_drv)){;}
    explicit BMI088_Gyr(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        phy_(hal::SpiDrv{spi, index}){;}


    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<Vec3<q24>> read_gyr();


    [[nodiscard]] IResult<> set_gyr_fs(const GyrFs range);
    [[nodiscard]] IResult<> set_gyr_odr(const GyrOdr odr);
private:
    BoschSensor_Phy phy_;
    real_t gyr_scaler_ = 0;

    BMI088_GyrRegs regs_ = {};

    [[nodiscard]] IResult<> verify_chip_id();

    [[nodiscard]] static constexpr Option<real_t> 
    calculate_gyr_scale(const GyrFs range){
        switch(range){
            default:
                return None;
            case GyrFs::_125deg:
                return Some(DEG2RAD<real_t>(150));
            case GyrFs::_250deg:
                return Some(DEG2RAD<real_t>(250));
            case GyrFs::_500deg:
                return Some(DEG2RAD<real_t>(500));
            case GyrFs::_1000deg:
                return Some(DEG2RAD<real_t>(1000));
            case GyrFs::_2000deg:
                return Some(DEG2RAD<real_t>(2000));
        }
    }
};
    

};

