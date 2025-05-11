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

    IResult<> init();
    
    IResult<> update();

    IResult<> validate();

    IResult<> reset();

    Option<Vector3_t<real_t>> get_acc();
    Option<Vector3_t<real_t>> get_gyr();
private:
    InvensenseSensor_Phy phy_;

    real_t lsb_acc_x64;
    real_t lsb_gyr_x256;

    #pragma pack(push, 1)
    Vector3_t<int16_t> acc_data_;
    Vector3_t<int16_t> gyr_data_;
    #pragma pack(pop)

    static_assert(sizeof(acc_data_) == 6);
    static_assert(sizeof(gyr_data_) == 6);
};

}
