#pragma once

#include "../constants.hpp"
#include "../observer/observer.hpp"

#ifdef DEBUG
#define CURR_SPEC public
#define POS_SPEC public
#define SPD_SPEC public
#else
#define CURR_SPEC protected
#define POS_SPEC protected
// #define SPD_SPEC protected
#define SPD_SPEC public
#endif

struct CtrlResult{
    real_t current;
    real_t raddiff;
};

struct CtrlLimits{
    Range pos_limit;
    real_t max_curr;
    real_t max_spd;
    int max_acc;

    void reset();
};


struct CurrentCtrl{
using Result = CtrlResult;
public:
    struct Config{
        real_t curr_slew_rate;
        real_t rad_slew_rate;
        real_t openloop_curr;

        void reset();
    };

    CtrlLimits & limits;
    Config & config;

CURR_SPEC:
    bool ctrl_done = false;
public:
    real_t curr_output = 0;
    real_t raddiff_output = 0;
    CurrentCtrl(CtrlLimits & _limits, Config & _config):limits(_limits), config(_config){reset();}

    void reset(){
        config.reset();
        curr_output = 0;
        raddiff_output = 0;
    }

    operator bool() const {
        return ctrl_done; 
    }
    
    CtrlResult update(const CtrlResult & result);
    CtrlResult output() const {return {curr_output, raddiff_output};}

    real_t getLastCurrent() const {return curr_output; }
    real_t getLastRaddiff() const {return raddiff_output;}
};



struct HighLayerCtrl{
protected:
    CtrlLimits & limits;
    CurrentCtrl & curr_ctrl;
    using Result = CtrlResult;
public:
    HighLayerCtrl(CtrlLimits & _limits, CurrentCtrl & _ctrl):limits(_limits), curr_ctrl(_ctrl){;}
    virtual void reset() = 0;
};

struct SpeedCtrl:public HighLayerCtrl{
public:
    struct Config{
        real_t kp;
        real_t kp_limit;

        real_t kd;
        real_t kd_limit;

        void reset();
    };
    
    Config & config;
SPD_SPEC:
    real_t last_real_spd = 0;
    real_t soft_targ_spd = 0;
    real_t filt_real_spd = 0;
public:
    SpeedCtrl(CtrlLimits & _limits, Config & _config, CurrentCtrl & _curr_ctrl):
        HighLayerCtrl(_limits, _curr_ctrl), config(_config){reset();}

    void reset() override {
        config.reset();
        last_real_spd = 0;
        soft_targ_spd = 0;
    }

    Result update(const real_t _targ_speed,const real_t real_speed);
};

struct PositionCtrl:public HighLayerCtrl{
public:
    struct Config{
        real_t kp;
        real_t ki;
        real_t kd;

        void reset();
    };

    Config & config;
protected:
    SpeedEstimator::Config spe_config;
    SpeedEstimator targ_spd_est{spe_config};

    real_t ki_integral = 0;
    real_t targ_spd = 0;
public:
    PositionCtrl(CtrlLimits & _limits, Config & _config, CurrentCtrl & _curr_ctrl):
        HighLayerCtrl(_limits, _curr_ctrl), config(_config){reset();}

    void reset() override {
        config.reset();
        targ_spd_est.reset();
        ki_integral = 0;
        targ_spd = 0;
    }

    operator bool () const {
        return ki_integral == 0;
    }

    Result update(const real_t targ_position, const real_t real_position, 
            const real_t real_speed, const real_t real_elecrad);
};

struct TopLayerCtrl{
    CtrlLimits & limits;
    SpeedCtrl & speed_ctrl;
    PositionCtrl & position_ctrl;
};

struct TrapezoidPosCtrl:public TopLayerCtrl{
public:
    struct Config{
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
    TrapezoidPosCtrl(CtrlLimits & _limits, Config & _config, SpeedCtrl & _speed_ctrl, PositionCtrl & _position_ctrl):
            TopLayerCtrl(_limits, _speed_ctrl, _position_ctrl), config(_config){
                reset();
            }
    
    void reset(){
        config.reset();
        goal_speed = 0;
        // last_pos_err = 0;    
    }
    Result update(const real_t targ_position,const real_t real_position, const real_t real_speed, const real_t real_elecrad);
};