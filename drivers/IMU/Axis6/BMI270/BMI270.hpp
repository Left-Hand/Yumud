#pragma once

#include "bmi270_prelude.hpp"

namespace ymd::drivers{


class BMI270 final:
    public AccelerometerIntf, 
    public GyroscopeIntf,
    private BMI270_Regs{
public:
    using Error = BMI270_Prelude::Error;

    explicit BMI270(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        phy_(hal::I2cDrv{i2c, DEFAULT_I2C_ADDR}){;}


    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<> set_pmu_mode(const PmuType pum, const PmuMode mode);
    [[nodiscard]] PmuMode get_pmu_mode(const PmuType pum);
    [[nodiscard]] IResult<Vec3<q24>> read_acc();
    [[nodiscard]] IResult<Vec3<q24>> read_gyr();
private:
    BoschSensor_Phy phy_;

};

}
