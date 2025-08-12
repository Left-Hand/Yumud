#pragma once


#include "motion_module.hpp"
#include "../autodrive/Filters.hpp"
#include "wheel/wheels.hpp"


namespace ymd::robots{

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
public:
    PositionCtrl(const Config & config):
        config_(config){;}

    PositionCtrl(const PositionCtrl & other) = delete;
    PositionCtrl(PositionCtrl && other) = delete;

    Vec2<q16> update(const Vec2<q16> & targ_pos, const Vec2<q16> & pos, const Vec2<q16> & spd);
};


class FeedBacker{
public:
    virtual Vec2<q16> pos() = 0;
    virtual real_t rad() = 0;
};


class FeedBackerOpenLoop:public FeedBacker{

};

class FeedBackerCloseLoop:public FeedBacker{

};


}