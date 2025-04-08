#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "drivers/IMU/details/BoschIMU.hpp"


namespace ymd::drivers{

class BMM150:public Magnetometer{
public:
    using Error = details::BoschSensorError;

protected:
    BoschSensor_Phy phy_;
    using RegAddress = uint8_t;

    scexpr uint8_t default_i2c_addr = 0x68;

    struct{

    };


public:
    BMM150(hal::I2c & bus, const uint8_t addr = default_i2c_addr):phy_(hal::I2cDrv(bus, addr)){;}


    void init();
    void update();

    bool verify();

    void reset();

    Option<Vector3_t<real_t>> get_magnet() override;
};


}