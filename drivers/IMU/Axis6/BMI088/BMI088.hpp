
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
        transport_(i2c_drv){;}
    explicit BMI088_Acc(hal::I2cDrv && i2c_drv):
        transport_(std::move(i2c_drv)){;}
    explicit BMI088_Acc(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        transport_(hal::I2cDrv{i2c, DEFAULT_I2C_ADDR}){;}

    explicit BMI088_Acc(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit BMI088_Acc(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit BMI088_Acc(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(hal::SpiDrv{spi, rank}){;}


    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<Vec3<iq24>> read_acc();
    [[nodiscard]] IResult<iq16> read_temp();

    [[nodiscard]] IResult<> set_acc_fs(const AccFs gyr_fs);
    [[nodiscard]] IResult<> set_acc_bwp(const AccBwp bwp);
    [[nodiscard]] IResult<> set_acc_odr(const AccOdr odr);
private:
    BoschImu_Transport transport_;
    iq20 acc_scaler_ = 0;
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
    //         return bmi_.transport_.write_reg(reg);
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

    [[nodiscard]] static constexpr iq20
    calculate_acc_scale(const AccFs acc_fs){
        constexpr double g = 9.806;
        switch(acc_fs){
            case AccFs::_3G:
                return iq20(g * 3);
            case AccFs::_6G:
                return iq20(g * 6);
            case AccFs::_12G:
                return iq20(g * 12);
            case AccFs::_24G:
                return iq20(g * 24);
        }

        __builtin_unreachable();
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
        transport_(i2c_drv){;}
    explicit BMI088_Gyr(hal::I2cDrv && i2c_drv):
        transport_(std::move(i2c_drv)){;}
    explicit BMI088_Gyr(Some<hal::I2cBase *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        transport_(hal::I2cDrv{i2c, DEFAULT_I2C_ADDR}){;}

    explicit BMI088_Gyr(const hal::SpiDrv & spi_drv):
        transport_(spi_drv){;}
    explicit BMI088_Gyr(hal::SpiDrv && spi_drv):
        transport_(std::move(spi_drv)){;}
    explicit BMI088_Gyr(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        transport_(hal::SpiDrv{spi, rank}){;}


    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> reset();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<Vec3<iq24>> read_gyr();


    [[nodiscard]] IResult<> set_gyr_fs(const GyrFs gyr_fs);
    [[nodiscard]] IResult<> set_gyr_odr(const GyrOdr odr);
private:
    BoschImu_Transport transport_;
    iq20 gyr_scaler_ = 0;

    BMI088_GyrRegs regs_ = {};

    [[nodiscard]] IResult<> verify_chip_id();

    [[nodiscard]] static constexpr iq20
    calculate_gyr_scale(const GyrFs gyr_fs){
        switch(gyr_fs){
            case GyrFs::_125deg:
                return iq24(DEG2RAD_RATIO) * (2 * 125);
            case GyrFs::_250deg:
                return iq24(DEG2RAD_RATIO) * (2 * 250);
            case GyrFs::_500deg:
                return iq24(DEG2RAD_RATIO) * (2 * 500);
            case GyrFs::_1000deg:
                return iq24(DEG2RAD_RATIO) * (2 * 1000);
            case GyrFs::_2000deg:
                return iq24(DEG2RAD_RATIO) * (2 * 2000);
        }

        __builtin_unreachable();
    }
};
    

};

