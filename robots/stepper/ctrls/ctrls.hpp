#pragma once

#include "../constants.hpp"

#ifdef DEBUG
#define CURR_SPEC public
#define POS_SPEC public
#define SPD_SPEC public
#else
#define CURR_SPEC protected
#define POS_SPEC protected
#define SPD_SPEC protected
#endif

struct CtrlResult{
    real_t current;
    real_t raddiff;
};


struct CurrentCtrl{
using Result = CtrlResult;
public:
    struct Config{
        real_t current_slew_rate;   //20A/S
        real_t current_clamp;

        constexpr void reset(){
            current_slew_rate = 30.0 / foc_freq;
            current_clamp = 0.7;
        }
    };

    const Config & config;

CURR_SPEC:
    const real_t & current_slew_rate = config.current_slew_rate;
    const real_t & current_clamp = config.current_clamp;
public:
    real_t current_output = 0;

    CurrentCtrl(const Config & _config):config(_config){}

    void init(){
        reset();
    }

    void reset(){
        current_output = 0;
    }

    real_t update(const real_t targ_current);
};



struct HighLayerCtrl{
protected:
    CurrentCtrl & curr_ctrl;
    using Result = CtrlResult;
public:

    HighLayerCtrl(CurrentCtrl & _ctrl):curr_ctrl(_ctrl){;}
    virtual void reset() = 0;
};

struct SpeedCtrl:public HighLayerCtrl{
    SpeedCtrl(CurrentCtrl & ctrl):HighLayerCtrl(ctrl){;}
    virtual Result update(const real_t targ_speed,const real_t real_speed) = 0;
};

struct PositionCtrl:public HighLayerCtrl{
    PositionCtrl(CurrentCtrl & ctrl):HighLayerCtrl(ctrl){;}

    virtual Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad) = 0;
};

struct GeneralSpeedCtrl:public SpeedCtrl{
public:
    struct Config{
        real_t max_spd = 40;
        real_t kp = 4;
        real_t kp_clamp = 60;

        real_t kd = 40;
        real_t kd_active_radius = 1.2;
        real_t kd_clamp = 2.4;
    };
    
    const Config & config;
SPD_SPEC:
    const real_t & kp = config.kp;
    const real_t & kp_clamp = config.kd_clamp;

    const real_t & kd = config.kd;
    const real_t & kd_active_radius = config.kd_active_radius;
    const real_t & kd_clamp = config.kd_clamp;

    real_t targ_current = 0;
    real_t last_speed = 0;

public:
    const real_t & max_spd = config.max_spd;
    GeneralSpeedCtrl(CurrentCtrl & ctrl, const Config & _config):SpeedCtrl(ctrl), config(_config){;}
    void reset() override {
        targ_current = 0;
    }

    Result update(const real_t _targ_speed,const real_t real_speed);
};

struct GeneralPositionCtrl:public PositionCtrl{
public:
    struct Config{
        real_t kp = 60;
        real_t kd = 30;

        real_t kd_active_radius = 1.7;

        real_t ki = 0.0;
        real_t ki_clamp = 25.6;
    };

    const Config & config;

POS_SPEC:
    const real_t & kp = config.kp;
    const real_t & kd = config.kd;
    const real_t & kd_active_radius = config.kd_active_radius;

    const real_t & ki = config.ki;
    const real_t & ki_clamp = config.ki_clamp;

    real_t intergalx256;
    real_t last_error;
    real_t last_current;
    real_t error;
public:
    GeneralPositionCtrl(CurrentCtrl & ctrl, const Config & _config):PositionCtrl(ctrl), config(_config){;}

    void reset() override {
        intergalx256 = 0;
    }

    Result update(const real_t targ_position, const real_t real_position, 
            const real_t real_speed, const real_t real_elecrad);
};

struct TopLayerCtrl{
    GeneralSpeedCtrl & speed_ctrl;
    GeneralPositionCtrl & position_ctrl;
};

struct TrapezoidPosCtrl:public TopLayerCtrl{
public:
    struct Config{
        real_t max_dec = 18.0;
        real_t pos_sw_radius = 0.02;
    };

    using Result = CtrlResult;

    const Config & config;
protected:
    const real_t & max_dec = config.max_dec;
    const real_t & pos_sw_radius = config.pos_sw_radius;

    enum class Tstatus:uint8_t{
        ACC,
        DEC,
        STA,
    };

    Tstatus tstatus = Tstatus::STA;
    real_t goal_speed = 0;
    real_t last_pos_err = 0;


public:
    TrapezoidPosCtrl(GeneralSpeedCtrl & _speed_ctrl, GeneralPositionCtrl & _position_ctrl, const Config & _config):TopLayerCtrl(_speed_ctrl, _position_ctrl), config(_config){;}
    Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad);
};