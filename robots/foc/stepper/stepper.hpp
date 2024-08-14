#ifndef __STEPPER_HPP__

#define __STEPPER_HPP__

#include "cli.hpp"
#include "ctrls/ctrls.hpp"
#include "observer/observer.hpp"
#include "archive/archive.hpp"

#include "robots/foc/focmotor.hpp"
#include "robots/foc/protocol/ascii_protocol.hpp"
#include "robots/foc/protocol/can_protocol.hpp"

#include "hal/timer/pwm/gpio_pwm.hpp"

class FOCStepper:public FOCMotor{
    using StatLed = StepperComponents::StatLed;
    using Archive = StepperUtils::Archive;
    using Switches = StepperUtils::Switches;

    using NodeId = StepperUtils::NodeId;


    #define THROW_ERROR(code, msg) throw_error(code,msg)
    #define THROW_WARN(code, msg) throw_warn(code,msg)


    #ifdef STEPPER_NO_PRINT
    // #define CLI_PRINTS(...)
    #define ARCHIVE_PRINTS(...)
    #define CLI_DEBUG(...)
    #define COMMAND_DEBUG(...)
    #define RUN_DEBUG(...)

    #else
    // #define CLI_PRINTS(...) logger.prints(__VA_ARGS__);

    #define ARCHIVE_PRINTS(...) if(outen) DEBUGGER.prints(__VA_ARGS__);

    #define CALI_DEBUG(...)\
    if(cali_debug_enabled){\
    logger.println(__VA_ARGS__);};

    #define COMMAND_DEBUG(...)\
    if(command_debug_enabled){\
    logger.println(__VA_ARGS__);};

    #define RUN_DEBUG(...)\
    if(run_debug_enabled){\
    logger.println(__VA_ARGS__);};

    #endif

    Archive archive_;
    Switches & switches_ = archive_.switches;

    volatile CtrlType ctrl_type = CtrlType::POSITION;

    GpioPwm red_pwm{portC[14]};
    GpioPwm green_pwm{portC[15]};
    GpioPwm blue_pwm{portC[13]};
    RgbLedAnalog rgb_led{red_pwm, green_pwm, blue_pwm};
    StatLed panel_led = StatLed{rgb_led, run_status, ctrl_type};

    real_t elecrad_zerofix;
    real_t run_elecrad;
    real_t est_elecrad;
    real_t run_leadangle;

    CurrentCtrl::Config curr_config;
    CurrentCtrl curr_ctrl{ctrl_limits, curr_config};
    
    SpeedCtrl::Config spd_config;
    SpeedCtrl speed_ctrl{ctrl_limits, spd_config, curr_ctrl};
    
    PositionCtrl::Config pos_config;
    PositionCtrl position_ctrl{ctrl_limits, pos_config, curr_ctrl};

    TrapezoidPosCtrl::Config tpz_config;
    TrapezoidPosCtrl trapezoid_ctrl{ctrl_limits, tpz_config, speed_ctrl, position_ctrl};

    SpeedEstimator::Config spe_config;
    SpeedEstimator speed_estmator{spe_config};

    bool cali_debug_enabled = true;
    bool command_debug_enabled = false;
    bool run_debug_enabled = false;
    
    uint64_t exe_micros = 0;

    bool skip_tone = false;
    bool cmd_mode = false;

    RunStatus cali_task(const InitFlag init_flag = false);
    RunStatus active_task(const InitFlag init_flag = false);
    RunStatus beep_task(const InitFlag init_flag = false);
    RunStatus check_task(const InitFlag init_flag = false);

    friend class AsciiProtocol;
    friend class CanProtocol;
public:
    FOCStepper(SVPWM2 & _svpwm, Encoder & _encoder, Memory & _memory):
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

    void setNozzle(const real_t duty);
    void tick();


    void init(){
        ctrl_limits.reset();
        curr_config.reset();
        
        odo.init();

        panel_led.init();

        red_pwm.setPeriod(25);
        green_pwm.setPeriod(25);
        blue_pwm.setPeriod(25);
    }

    void setTargetCurrent(const real_t curr){
        target = MIN(ctrl_limits.max_curr, curr);
        ctrl_type = CtrlType::CURRENT;
    }

    void setTargetSpeed(const real_t speed){
        target = MIN(ctrl_limits.max_spd, speed);
        ctrl_type = CtrlType::SPEED;
    }

    void setTargetPosition(const real_t pos){
        target = ctrl_limits.pos_limit.clamp(pos);
        ctrl_type = CtrlType::POSITION;
    }

    void setTargetTrapezoid(const real_t pos){
        target = ctrl_limits.pos_limit.clamp(pos);
        ctrl_type = CtrlType::TRAPEZOID;
    }

    void setTargetTeach(const real_t max_curr){
        target = CLAMP(max_curr, 0, ctrl_limits.max_curr);
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
        ctrl_limits.max_curr = current;
    }

    void locateRelatively(const real_t pos = 0){
        odo.locateRelatively(pos);
    }

    void run();

    void report();





    void setPositionLimit(const Range & clamp){
        ctrl_limits.pos_limit = clamp;
    }

    void enable(const bool en = true){
        if(en){
            rework();
        }else{
            shutdown();
        }
    }



    real_t getPositionErr(){
        return getPosition() - target;
    }

    real_t getSpeedErr(){
        return getSpeed() - target;
    }


    void setSpeedLimit(const real_t max_spd){
        ctrl_limits.max_spd = max_spd;
    }

    void setAccelLimit(const real_t max_acc){
        ctrl_limits.max_acc = int(max_acc);
    }

    void triggerCali(){
        cali_task(true);
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

#endif