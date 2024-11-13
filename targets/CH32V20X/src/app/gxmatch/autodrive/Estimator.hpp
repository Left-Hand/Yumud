#pragma once

#include "drivers/IMU/IMU.hpp"
#include "Gesture2D.hpp"

namespace gxm{

using namespace yumud::drivers;
// using Accelerometer = yumud::Accelerometer;

class Estimator{
public:


    struct Config{
        Vector3 acc_trim;
        Vector3 gyro_trim;

        size_t trim_times;
    };
protected:
    const Config & config_;

    Accelerometer & acc_sensor_;
    Gyroscope & gyro_sensor_;
    Magnetometer & mag_sensor_;

    Vector3 acc3_;
    Vector3 gyro3_;

    Vector2 acc_;
    real_t gyro_;
public:
    Estimator(const Config & config, Accelerometer & acc_sensor, Gyroscope & gyro_sensor, Magnetometer & mag_sensor):
        config_(config), acc_sensor_(acc_sensor), gyro_sensor_(gyro_sensor), mag_sensor_(mag_sensor){;}

    void update();

    Vector3 calculateAccelTrim();
    Vector3 calculateGyroTrim();

    auto acc() const{return acc_;}
    auto gyro() const{return gyro_;}
    
    auto acc3() const{return acc3_;}
    auto gyro3() const{return gyro3_;}

    auto gest() const {
        
    }
};


}