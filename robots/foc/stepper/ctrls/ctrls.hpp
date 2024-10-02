#pragma once

#include "../motor_utils.hpp"
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


struct CurrentCtrl{
using Result = CtrlResult;
public:
    struct Config{
        real_t curr_slew_rate;
        real_t rad_slew_rate;
        void reset();
    };

    MetaData & meta;
    Config & config;

protected:
    real_t last_curr = 0;
    real_t last_raddiff = 0;
public:

    CurrentCtrl(MetaData & _meta, Config & _config):meta(_meta), config(_config){reset();}

    void reset(){
        config.reset();
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

CtrlResult CurrentCtrl::update(const CtrlResult res){
    last_curr = STEP_TO(last_curr, res.current, config.curr_slew_rate);
    last_raddiff = STEP_TO(last_raddiff, res.raddiff, config.rad_slew_rate);

    return {last_curr, last_raddiff};
}


struct HighLayerCtrl{
protected:
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
    SpeedCtrl(MetaData & _meta, Config & _config):
        HighLayerCtrl(_meta), config(_config){reset();}

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
    SpeedEstimator targ_spd_est;
public:
    PositionCtrl(MetaData & _meta, Config & _config, const SpeedEstimator::Config & _tspe_config):
        HighLayerCtrl(_meta),
        config(_config),
        targ_spd_est(_tspe_config)
        {reset();}

    void reset() override {
        config.reset();
        targ_spd_est.reset();
    }

    Result update(const real_t targ_position, const real_t real_position, 
            const real_t real_speed);
};
