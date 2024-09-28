#pragma once

#include "hal/timer/pwm/gpio_pwm.hpp"

#include "robots/foc/focmotor.hpp"

#include "drivers/Actuator/SVPWM/svpwm2.hpp"

#include "ctrls/ctrls.hpp"
#include "observer/observer.hpp"
#include "archive/archive.hpp"

#include "tasks/cali.hpp"
#include "tasks/tone.hpp"
#include "tasks/selfcheck.hpp"

#include "statled.hpp"



class FOCStepper:public FOCMotor{
    using StatLed = StepperComponents::StatLed;
    using Archive = MotorUtils::Archive;
    using Switches = MotorUtils::Switches;

    // using NodeId = MotorUtils::NodeId;

    Archive archive_;
    Switches & switches_ = archive_.switches;

    volatile CtrlType ctrl_type = CtrlType::POSITION;

    GpioPwm red_pwm{portC[14]};
    GpioPwm green_pwm{portC[15]};
    GpioPwm blue_pwm{portC[13]};
    RgbLedAnalog rgb_led{red_pwm, green_pwm, blue_pwm};
    StatLed panel_led = StatLed{rgb_led, run_status, ctrl_type};

    CurrentCtrl::Config curr_config;
    CurrentCtrl curr_ctrl{meta, curr_config};
    
    SpeedCtrl::Config spd_config;
    SpeedCtrl speed_ctrl{meta, spd_config, curr_ctrl};

    PositionCtrl::Config pos_config;
    PositionCtrl position_ctrl{meta, pos_config, curr_ctrl};
    
    TrapezoidPosCtrl::Config tpz_config;
    TrapezoidPosCtrl trapezoid_ctrl{meta, tpz_config, speed_ctrl, position_ctrl};

    SpeedEstimator::Config spe_config;
    SpeedEstimator speed_estmator{spe_config};

    bool cali_debug_enabled = true;
    bool command_debug_enabled = false;
    bool run_debug_enabled = false;
    
    uint64_t exe_micros = 0;

    bool skip_tone = false;
    bool cmd_mode = false;

    CaliTasker cali_tasker = {svpwm, odo};
    ToneTasker tone_tasker{svpwm};

    void invoke_cali();
    void invoke_tone_task();
    void invoke_active_task();
    
    void active_task();

    SelfCheckTasker selfcheck_tasker{svpwm, odo};
    void invoke_selfcheck_task();
    
    friend class AsciiProtocol;
    friend class CanProtocol;
public:
    FOCStepper(SVPWM & _svpwm, Encoder & _encoder, Memory & _memory):
            FOCMotor(_svpwm, _encoder, _memory){;}

    bool isActive() const {
        return (RunStatus::ACTIVE) == run_status;
    }

    volatile RunStatus & status(){
        return run_status;
    }

    bool loadArchive(const bool outen = false);
    void saveArchive(const bool outen = false);
    void removeArchive(const bool outen = false);

    virtual real_t getTarget(){return target;}

    void tick();

    void init(){
        meta.reset();
        curr_config.reset();
        
        odo.init();

        panel_led.init();

        red_pwm.setPeriod(25);
        green_pwm.setPeriod(25);
        blue_pwm.setPeriod(25);
    }

    void setTargetCurrent(const real_t curr){
    target = MIN(curr, meta.max_curr);
        ctrl_type = CtrlType::CURRENT;
    }

    void setTargetSpeed(const real_t speed){
        target = MIN(speed, meta.max_spd);
        ctrl_type = CtrlType::SPEED;
    }

    void setTargetPosition(const real_t pos){
        target = meta.pos_limit.clamp(pos);
        ctrl_type = CtrlType::POSITION;
    }

    void setTargetTrapezoid(const real_t pos){
        target = meta.pos_limit.clamp(pos);
        ctrl_type = CtrlType::TRAPEZOID;
    }

    void setTargetTeach(const real_t max_curr){
        target = CLAMP(max_curr, 0, meta.max_curr);
        ctrl_type = CtrlType::TEACH;
    }

    void setOpenLoopCurrent(const real_t current){
        curr_config.openloop_curr = current;
    }

    void setTargetVector(const real_t pos){
        target = pos;
        ctrl_type = CtrlType::VECTOR;
    }

    void freeze(){
        setTargetPosition(getPosition());
    }

    void setCurrentLimit(const real_t current){
        meta.max_curr = current;
    }

    void locateRelatively(const real_t pos = 0){
        odo.locateRelatively(pos);
    }

    void run();

    void report();

    void setPositionLimit(const Range & clamp){
        meta.pos_limit = clamp;
    }

    void enable(const bool en = true){
        if(en){
            rework();
        }else{
            shutdown();
        }
    }

    uint32_t exe() const override {
        return exe_micros;
    }


    real_t getPositionErr(){
        return getPosition() - target;
    }

    real_t getSpeedErr(){
        return getSpeed() - target;
    }


    void setSpeedLimit(const real_t max_spd){
        meta.max_spd = int(max_spd);
    }

    void setAccelLimit(const real_t max_acc){
        meta.max_acc = int(max_acc);
    }

    void triggerCali(){
        invoke_cali();
    }

    void clear(){
        removeArchive();
    }

    void reset()override{
        Sys::Misc::reset();
    }

    void shutdown(){
        run_status = RunStatus::INACTIVE;
        svpwm.enable(false);
    }

    void rework(){
        run_status = RunStatus::ACTIVE;
        svpwm.enable(true);
    }
};