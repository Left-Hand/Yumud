#pragma once

#include "bmi270_prelude.hpp"

namespace ymd::drivers{


class BMI270 final:
    public BMI270_Prelude,
    public AccelerometerIntf{
public:
    using Error = BMI270_Prelude::Error;

    explicit BMI270(const hal::I2cDrv & i2c_drv): 
        phy_(i2c_drv){;}

    explicit BMI270(hal::I2cDrv && i2c_drv): 
        phy_(std::move(i2c_drv)){;}

    explicit BMI270(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        phy_(hal::I2cDrv(i2c, addr)){;}

    explicit BMI270(const hal::SpiDrv & spi_drv): 
        phy_(spi_drv){;}

    explicit BMI270(hal::SpiDrv && spi_drv): 
        phy_(std::move(spi_drv)){;}

    explicit BMI270(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank): 
        phy_(hal::SpiDrv{spi, rank}){;}

    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_pmu_mode(const PmuType pmu, const PmuMode mode);
    [[nodiscard]] PmuMode get_pmu_mode(const PmuType pmu);
    [[nodiscard]] IResult<Vec3<iq24>> read_acc();
private:
    BoschSensor_Phy phy_;
    BMI270_Regset regs_ = {};
};

}
