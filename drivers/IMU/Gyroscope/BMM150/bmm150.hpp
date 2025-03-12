#pragma once

#include <optional>

#include "drivers/device_defs.h"
#include "drivers/IMU/BoschIMU.hpp"


namespace ymd::drivers{

class BMM150:public Magnetometer, public BoschSensor{
public:


protected:

    using RegAddress = uint8_t;

    scexpr uint8_t default_i2c_addr = 0x68;

    struct{

    };



    void writeCommand(const uint8_t cmd){
        writeReg(0x7e, cmd);
    }
public:
    using BoschSensor::BoschSensor;
    BMM150(hal::I2c & bus, const uint8_t addr = default_i2c_addr):BoschSensor(hal::I2cDrv(bus, addr)){;}


    void init();
    void update();

    bool verify();

    void reset();

    Option<Vector3> getMagnet() override;
};


}