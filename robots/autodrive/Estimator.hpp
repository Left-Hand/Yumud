#pragma once

#include "drivers/IMU/IMU.hpp"
#include "drivers/Proximeter/FlowSensor.hpp"

namespace gxm{

using namespace ymd::drivers;

// using Vector2Lpf = LowpassFilter<Vector2>;

// class SpeedObserver{
// public:
//     struct Config{
//         real_t kq;
//     };
    
// protected:
//     real_t rot_;
//     real_t rot_unfiltered;
//     real_t last_rot;
//     real_t last_gyr;
//     real_t last_time;
    
//     bool inited;
//     const Config & config_;
// public:
//     SpeedObserver(const Config & config):
//         config_(config){
//             reset();
//         }

//     real_t update(const real_t rot, const real_t gyr, const real_t time){

//         do{
//             if(!inited){
//                 rot_ = rot;
//                 rot_unfiltered = rot;
//                 inited = true;
//             }else{
//                 const real_t delta_t = (time - last_time);
//                 rot_unfiltered += gyr * delta_t;
//                 rot_unfiltered = config_.kq * rot_ + (1-config_.kq) * rot;
//                 rot_ = config_.ko * rot_ + (1-config_.ko) * rot_unfiltered;
//             }
//         }while(false);
        
//         last_rot = rot;
//         last_gyr = gyr;
//         last_time = time;
//         return {rot_};
//     }

//     void reset(const real_t time = 0){
//         rot_ = 0;
//         rot_unfiltered = 0;
//         last_rot = 0;
//         last_gyr = 0;
//         last_time = time;
//         inited = false;
//     }
// };




// //底盘观测器 用于从惯性计与里程计估测姿态
// class Estimator{
// public:
//     using Mahony = ymd::Mahony;
//     //偏置
//     struct Bias{
//         Quat acc;
//         Vector3 gyr;
//         Quat mag;
//         real_t rot;
//     };

//     struct Config{
//         size_t calibrate_times;//校准次数
//         bool force_calibrate;//强制校准而不是使用先验偏置
//         Bias pre_bias;//偏置

//         ComplementaryFilter::Config rot_obs_config;
//         Vector2Lpf::Config spd_lpf_config;
//     };
// // protected:
// public:
//     const Config & config_;

//     Axis6 & acc_gyr_sensor_;//加速度与角速度计
//     MagnetometerIntf & mag_sensor_;//地磁计
//     FlowSensor & flow_sensor_;

//     // ComplementaryFilter rot_obs_{config_.rot_obs_config};
//     RotationObserver rot_obs_{config_.rot_obs_config};
//     Vector2Lpf spd_lpf_{config_.spd_lpf_config};
    
//     Bias bias_;

//     Vector3 acc3_raw;
//     Vector3 gyr3_raw;
//     Vector3 mag3_raw;

//     Vector3 acc3_;
//     Vector3 gyr3_;
//     Vector3 mag3_;

//     Vector2 acc_;
//     real_t gyr_;
//     real_t rot_;
    
//     Vector3 euler_;

//     Vector2 pos_;
//     Vector2 spd_;

//     real_t last_time;

//     real_t calculate_raw_dir(const Vector3 & mag) const;
// public:
//     Estimator(const Estimator & other) = delete;
//     Estimator(Estimator && other) = delete;

//     Estimator(const Config & config, 
//             Axis6 & acc_gyr_sensor, 
//             MagnetometerIntf & mag_sensor,
//             FlowSensor & flow_sensor):

//         config_(config), 
//         acc_gyr_sensor_(acc_gyr_sensor), 
//         mag_sensor_(mag_sensor),
//         flow_sensor_(flow_sensor){;}

//     void init();
//     void reset();
//     void calibrate();

//     void recalibrate(const Vector2 & _pos, const real_t _rot);
//     void update(const real_t time);

//     Quat calculateAccBias();
//     Vector3 calculateGyrBias();
//     Quat calculateMagBias();

//     auto acc() const{return acc_;}
//     auto gyr() const{return gyr_;}
    
//     auto acc3() const{return acc3_;}
//     auto gyr3() const{return gyr3_;}
//     auto rot() const{return rot_;}

//     auto pos() const{return pos_;}

//     auto spd() const{return spd_;}

//     auto euler() const{
//         return euler_;
//     }

// };


}