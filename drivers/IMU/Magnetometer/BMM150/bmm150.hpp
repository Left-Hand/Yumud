#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"


namespace ymd::drivers{

class BMM150:public MagnetometerIntf{
public:
    using Error = ImuError;

    template<typename T = void>
    using IResult = Result<T, Error>;

protected:
    BoschSensor_Phy phy_;
    using RegAddress = uint8_t;

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x68);

public:
    BMM150(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        phy_(hal::I2cDrv(i2c, addr)){;}


    void init();
    void update();

    bool validate();

    void reset();

    IResult<Vector3<q24>> read_mag();
};


}