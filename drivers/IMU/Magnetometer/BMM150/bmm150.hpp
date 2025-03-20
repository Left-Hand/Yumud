#pragma once

#include <optional>

#include "core/io/regs.hpp"
#include "drivers/IMU/BoschIMU.hpp"


namespace ymd::drivers{

class BMM150:public Magnetometer, public BoschSensor{
public:


protected:

    using RegAddress = uint8_t;

    scexpr uint8_t default_i2c_addr = 0x68;

    struct{

    };


public:
    using BoschSensor::BoschSensor;
    BMM150(hal::I2c & bus, const uint8_t addr = default_i2c_addr):BoschSensor(hal::I2cDrv(bus, addr)){;}


    void init();
    void update();

    bool verify();

    void reset();

    Option<Vector3_t<real_t>> getMagnet() override;
};


}