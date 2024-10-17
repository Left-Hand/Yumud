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

#include "robots/foc/components/statled/statled.hpp"



class FOCStepper:public FOCMotor{
    using StatLed = StepperComponents::StatLed;
    using Archive = MotorUtils::Archive;
    using Switches = MotorUtils::Switches;

    Archive archive_;
    Switches & switches_ = archive_.switches;

    volatile CtrlType ctrl_type = CtrlType::POSITION;

    GpioPwm red_pwm{portC[14]};
    GpioPwm green_pwm{portC[15]};
    GpioPwm blue_pwm{portC[13]};
    RgbLedAnalog rgb_led{red_pwm, green_pwm, blue_pwm};
    StatLed panel_led = StatLed{rgb_led, run_status, ctrl_type};

    CurrentFilter curr_ctrl{meta, archive_.curr_config};
    
    SpeedCtrl speed_ctrl{meta, archive_.spd_config};

    PositionCtrl position_ctrl{meta, archive_.pos_config};

    SpeedEstimator speed_estmator{archive_.spe_config};
    SpeedEstimator targ_spd_est{archive_.spe_config};

    // bool cali_debug_enabled = true;
    // bool command_debug_enabled = false;
    // bool run_debug_enabled = false;
    
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
    FOCStepper(const NodeId _id, SVPWM & _svpwm, Encoder & _encoder, Memory & _memory):
            FOCMotor(_id, _svpwm, _encoder, 50, _memory){;}

    bool isActive() const {
        return (RunStatus::ACTIVE) == run_status;
    }

    volatile RunStatus & status(){
        return run_status;
    }

    bool loadArchive();
    void saveArchive();
    void removeArchive();


    void tick();

    void init(){
        meta.reset();
        
        odo.init();

        panel_led.init();

        red_pwm.setPeriod(25);
        green_pwm.setPeriod(25);
        blue_pwm.setPeriod(25);
    }

    void setTargetCurrent(const real_t curr){
        meta.targ_curr = MIN(curr, meta.max_curr);
        ctrl_type = CtrlType::CURRENT;
    }

    void setTargetSpeed(const real_t speed){
        meta.targ_spd = MIN(speed, meta.max_spd);
        ctrl_type = CtrlType::SPEED;
    }

    void setTargetPosition(const real_t pos){
        meta.targ_pos = meta.pos_limit.clamp(pos);
        ctrl_type = CtrlType::POSITION;
    }

    void setTargetPositionDelta(const real_t delta){
        meta.targ_pos = meta.pos_limit.clamp(this->getPosition() + delta);
        ctrl_type = CtrlType::POSITION;
    }

    void setTargetTeach(const real_t max_curr){
        meta.targ_curr = CLAMP(max_curr, 0, meta.max_curr);
        ctrl_type = CtrlType::TEACH;
    }

    void setOpenLoopCurrent(const real_t current){
        meta.max_curr = current;
    }

    void setTargetVector(const real_t pos){
        meta.targ_pos = pos;
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

    real_t getRaddiff() const {
        return meta.raddiff;
    }

    void setRadfix(const real_t rf){
        meta.radfix = rf;
    }
};