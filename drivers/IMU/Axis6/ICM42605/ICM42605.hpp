//这个驱动尚未完成
//这个驱动尚未测试

#pragma once

#include "icm42605_prelude.hpp"
namespace ymd::drivers{

class ICM42605 final:
    public ICM42605_Prelude,
    public AccelerometerIntf, 
    public GyroscopeIntf
{
public:
    ICM42605(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        phy_(hal::I2cDrv(i2c, DEFAULT_I2C_ADDR)){;}

    [[nodiscard]] IResult<> init();
    
    [[nodiscard]] IResult<> validate();
    
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> reset();

    [[nodiscard]] IResult<Vec3<q24>> read_acc();
    [[nodiscard]] IResult<Vec3<q24>> read_gyr();

private:
    InvensenseSensor_Phy phy_;


    real_t lsb_acc_x64 = 0;
    real_t lsb_gyr_x256 = 0;

    
    Vec3<int16_t> acc_data_ = Vec3<int16_t>::ZERO;
    Vec3<int16_t> gyr_data_ = Vec3<int16_t>::ZERO;
};

}
