#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"


namespace ymd::drivers{

class BMM150:public MagnetometerIntf{
public:
    using Error = ImuError;

protected:
    BoschSensor_Phy phy_;
    using RegAddress = uint8_t;

    scexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x68);

    struct{

    };


public:
    BMM150(hal::I2c & bus, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):phy_(hal::I2cDrv(bus, addr)){;}


    void init();
    void update();

    bool validate();

    void reset();

    Option<Vector3_t<real_t>> get_magnet() override;
};


}