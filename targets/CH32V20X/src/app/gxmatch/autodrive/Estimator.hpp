#pragma once

#include "drivers/IMU/IMU.hpp"
#include "robots/gesture/mahony.hpp"
#include "Navigator.hpp"

namespace gxm{

using namespace ymd::drivers;
// using Accelerometer = ymd::Accelerometer;


//用于从惯性计与里程计估测姿态
class Estimator{
public:
    struct Bias{
        Quat acc;
        Vector3 gyro;
        Quat mag;
    };

    struct Config{
        size_t calibrate_times;
        bool force_calibrate;
        Bias pre_bias;
    };
protected:
    const Config & config_;

    Axis6 & acc_gyro_sensor_;
    Magnetometer & mag_sensor_;
    

    Bias bias_;

    Vector3 acc3_;
    Vector3 gyro3_;
    Vector3 mag3_;

    Vector2 acc_;
    real_t gyro_;
    real_t rot_;
    
    Vector3 euler_;
    Quat quat_;

    ymd::Mahony mahony_;
public:
    Estimator(const Config & config, Axis6 & acc_gyro_sensor, Magnetometer & mag_sensor):
        config_(config), acc_gyro_sensor_(acc_gyro_sensor), mag_sensor_(mag_sensor){;}

    void init();
    void calibrate();
    void update();
    void tick();

    Quat calculateAccelBias();
    Vector3 calculateGyroBias();
    Quat calculateMagBias();

    auto acc() const{return acc_;}
    auto gyro() const{return gyro_;}
    
    auto acc3() const{return acc3_;}
    auto gyro3() const{return gyro3_;}

    auto gest() const {
        
    }
};


}