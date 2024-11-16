#pragma once

#include "drivers/IMU/IMU.hpp"
#include "robots/gesture/mahony.hpp"
#include "Navigator.hpp"

namespace gxm{

using namespace ymd::drivers;
// using Accerometer = ymd::Accerometer;


//底盘观测器 用于从惯性计与里程计估测姿态
class Estimator{
public:

    //偏置
    struct Bias{
        Quat acc;
        Vector3 gyr;
        Quat mag;
    };

    struct Config{
        size_t calibrate_times;//校准次数
        bool force_calibrate;//强制校准而不是使用先验偏置
        Bias pre_bias;//偏置
    };
protected:
    const Config & config_;

    Axis6 & acc_gyr_sensor_;//加速度与角速度计
    Magnetometer & mag_sensor_;//地磁计
    

    Bias bias_;

    Vector3 acc3_;
    Vector3 gyr3_;
    Vector3 mag3_;

    Vector2 acc_;
    real_t gyr_;
    real_t rot_;
    
    Vector3 euler_;
    Quat quat_;

    ymd::Mahony mahony_;
public:
    Estimator(const Config & config, Axis6 & acc_gyr_sensor, Magnetometer & mag_sensor):
        config_(config), acc_gyr_sensor_(acc_gyr_sensor), mag_sensor_(mag_sensor){;}

    void init();
    void calibrate();
    void update();

    Quat calculateAccBias();
    Vector3 calculateGyrBias();
    Quat calculateMagBias();

    auto acc() const{return acc_;}
    auto gyr() const{return gyr_;}
    
    auto acc3() const{return acc3_;}
    auto gyr3() const{return gyr3_;}
    auto rot() const{return rot_;}
};


}