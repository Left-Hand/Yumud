#pragma once

#include "bmi270_prelude.hpp"

namespace ymd::drivers{


class BMI270 final:
    public BMI270_Prelude{
public:
    using Error = BMI270_Prelude::Error;

    explicit BMI270(const hal::I2cDrv & i2c_drv): 
        transport_(i2c_drv){;}

    explicit BMI270(hal::I2cDrv && i2c_drv): 
        transport_(std::move(i2c_drv)){;}

    explicit BMI270(
        Some<hal::I2cBase *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        transport_(hal::I2cDrv(i2c, addr)){;}

    explicit BMI270(const hal::SpiDrv & spi_drv): 
        transport_(spi_drv){;}

    explicit BMI270(hal::SpiDrv && spi_drv): 
        transport_(std::move(spi_drv)){;}

    explicit BMI270(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank): 
        transport_(hal::SpiDrv{spi, rank}){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_pmu_mode(const PmuType pmu, const PmuMode mode);
    [[nodiscard]] PmuMode get_pmu_mode(const PmuType pmu);
    [[nodiscard]] IResult<Vec3<iq24>> read_acc();
private:
    BoschImu_Transport transport_;
    BMI270_Regset regs_ = {};
};

}
