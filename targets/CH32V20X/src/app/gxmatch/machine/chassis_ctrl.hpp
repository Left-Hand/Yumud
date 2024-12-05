#pragma once


#include "motion_module.hpp"
#include "../autodrive/Estimator.hpp"
#include "wheel/wheels.hpp"
#include "wheel/wheel.hpp"


namespace gxm{

class ChassisModule;

class RotationCtrl{
public:
    struct Config{
        real_t kp;
        real_t kd;  
    };

protected:
    const Config & config_;
    // ChassisModule & chassis_;
public:
    RotationCtrl(const Config & config):
        config_(config){;}
        
    RotationCtrl(const RotationCtrl & other) = delete;
    RotationCtrl(RotationCtrl && other) = delete;

    real_t update(const real_t targ_rad, const real_t rad, const real_t gyr);
};

class ChassisModule;

class PositionCtrl{
public:
    struct Config{
        real_t kp;
        real_t kd;
    };

protected:
    const Config & config_;
    ChassisModule & chassis_;
public:
    PositionCtrl(const Config & config, ChassisModule & chassis):
        config_(config), chassis_(chassis){;}

    PositionCtrl(const PositionCtrl & other) = delete;
    PositionCtrl(PositionCtrl && other) = delete;

    void update(const Vector2 & targ_pos, const Vector2 & pos, const Vector2 & spd);
};

// class TravelCtrl{
// public:
//     struct Config{
//         real_t kp;
//         real_t kd;
//     };

// protected:
//     const Config & config_;
// public:
//     TravelCtrl(const Config & config):
//         config_(config), chassis_(chassis){;}

//     PositionCtrl(const PositionCtrl & other) = delete;
//     PositionCtrl(PositionCtrl && other) = delete;

//     // void update(const );
//     void reset();
// };

class FeedBacker{
public:
    virtual Vector2 pos() = 0;
    virtual real_t rad() = 0;
};


class FeedBackerOpenLoop:public FeedBacker{

};

class FeedBackerCloseLoop:public FeedBacker{

};


}