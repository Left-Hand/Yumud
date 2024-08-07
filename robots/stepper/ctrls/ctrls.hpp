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
        real_t curr_slew_rate;
        real_t rad_slew_rate;
        real_t curr_limit;
        real_t openloop_curr;

        void reset();
    };

    Config & config;

CURR_SPEC:
public:
    real_t current_output = 0;
    real_t raddiff_output = 0;
    CurrentCtrl(Config & _config):config(_config){reset();}

    void reset(){
        config.reset();
        current_output = 0;
        raddiff_output = 0;
    }

    CtrlResult update(CtrlResult result);
    CtrlResult output() const {return {current_output, raddiff_output};}
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
        real_t max_spd;
        real_t kp;
        real_t kp_limit;

        real_t kd;
        real_t kd_active_radius;
        real_t kd_limit;

        void reset();
    };
    
    Config & config;
SPD_SPEC:

    real_t targ_current = 0;
    real_t last_speed = 0;

public:
    const real_t & max_spd = config.max_spd;
    GeneralSpeedCtrl(CurrentCtrl & ctrl, Config & _config):
        SpeedCtrl(ctrl), config(_config){reset();}

    void reset() override {
        config.reset();
        targ_current = 0;
    }

    Result update(const real_t _targ_speed,const real_t real_speed);
};

struct GeneralPositionCtrl:public PositionCtrl{
public:
    struct Config{
        real_t kp;
        real_t kd;
        real_t kd2;

        void reset();
    };

    Config & config;

public:
    GeneralPositionCtrl(CurrentCtrl & ctrl, Config & _config):PositionCtrl(ctrl), config(_config){
        reset();
    }

    void reset() override {
        config.reset();
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
        int max_acc;
        int max_dec;
        real_t pos_sw_radius;

        void reset();
    };

    using Result = CtrlResult;

    Config & config;
protected:

    enum class Tstatus:uint8_t{
        ACC,
        DEC,
        STA,
    };

    Tstatus tstatus = Tstatus::STA;
    real_t goal_speed = 0;
    real_t last_pos_err = 0;


public:
    TrapezoidPosCtrl(GeneralSpeedCtrl & _speed_ctrl, GeneralPositionCtrl & _position_ctrl, Config & _config):
            TopLayerCtrl(_speed_ctrl, _position_ctrl), config(_config){
                reset();
            }
    
    void reset(){
        config.reset();
        goal_speed = 0;
        // last_pos_err = 0;    
    }
    Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad);
};