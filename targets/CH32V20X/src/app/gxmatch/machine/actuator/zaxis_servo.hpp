#pragma once

#include "zaxis.hpp"

namespace ymd::drivers{
class SpeedServo;
class DistanceSensor;
};

namespace gxm{
class ZAxisServo:public ZAxis{
public:
    struct Config:public ZAxis::Config{
        real_t kp;
        real_t kd;
    };
private:
    const Config & config_;
protected:
    using Motor = ymd::drivers::SpeedServo;
    using Sensor = ymd::drivers::DistanceSensor;
    Motor & motor_;
    Sensor & sensor_;
    real_t expect_distance = real_t(0.1);

    void setSpeed(const real_t speed);
    real_t getSpeed();
    real_t getErr();
public:
    ZAxisServo(const Config & config, Motor & motor, Sensor & sensor):
            ZAxis(config), config_(config), motor_(motor), sensor_(sensor){}
    DELETE_COPY_AND_MOVE(ZAxisServo)
    
    bool reached() override;
    void setDistance(const real_t dist) override;  
    real_t getDistance() override;
    void tick() override;

    void softHome() override;
};


}