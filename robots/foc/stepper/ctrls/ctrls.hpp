#pragma once
#include "robots/foc/motor_utils.hpp"
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


struct CurrentFilter{
using MetaData = MotorUtils::MetaData;
using Result = CtrlResult;
public:
    struct Config{
        real_t curr_slew_rate = real_t(60) / foc_freq;
        real_t rad_slew_rate = real_t(30) / foc_freq;
    };

    MetaData & meta;
    Config & config;

protected:
    real_t last_curr = 0;
    real_t last_raddiff = 0;
public:

    CurrentFilter(MetaData & _meta, Config & _config):meta(_meta), config(_config){reset();}

    void reset(){
        last_curr = 0;
        last_raddiff = 0;
    }
    
    __fast_inline CtrlResult update(const CtrlResult result);
    // CtrlResult output() const {return {curr_output, raddiff_output};}

    // real_t getLastCurrent() const {return curr_output; }
    real_t curr() const {return last_curr;}
    real_t raddiff() const {return last_raddiff;}
    // real_t getLastRaddiff() const {return raddiff_output;}
};

CtrlResult CurrentFilter::update(const CtrlResult res){
    last_curr = STEP_TO(last_curr, res.current, config.curr_slew_rate);
    last_raddiff = STEP_TO(last_raddiff, res.raddiff, config.rad_slew_rate);

    return {last_curr, last_raddiff};
}


struct HighLayerCtrl{
protected:
    using MetaData = MotorUtils::MetaData;
    MetaData & meta;
    using Result = CtrlResult;
public:
    HighLayerCtrl(MetaData & _meta):meta(_meta){;}
    virtual void reset() = 0;
};

struct SpeedCtrl:public HighLayerCtrl{
public:
    struct Config{
        real_t kp;

        real_t kd;
    };
    
    Config & config;
SPD_SPEC:
    real_t last_real_spd = 0;
    real_t soft_targ_spd = 0;
    real_t filt_real_spd = 0;
    real_t spd_delta = 0;
public:
    SpeedCtrl(MetaData & _meta, Config & _config):
        HighLayerCtrl(_meta), config(_config){reset();}

    void reset() override {
        last_real_spd = 0;
        soft_targ_spd = 0;
        filt_real_spd = 0;
        spd_delta = 0;
    }

    Result update(const real_t _targ_speed,const real_t real_speed);
};

struct PositionCtrl:public HighLayerCtrl{
public:
    struct Config{
        real_t kp;
        real_t kd;
    };

    Config & config;
protected:
    SpeedEstimator targ_spd_est;
    real_t last_targ_pos;
    bool locked;
public:
    PositionCtrl(MetaData & _meta, Config & _config, const SpeedEstimator::Config & _tspe_config):
        HighLayerCtrl(_meta),
        config(_config),
        targ_spd_est(_tspe_config)
        
        {
            reset();
        }

    void reset() override {
        targ_spd_est.reset();
    }

    Result update(
        const real_t targ_position, 
        const real_t real_position, 
        const real_t real_speed
    );
};
