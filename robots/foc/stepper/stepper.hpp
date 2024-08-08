#ifndef __STEPPER_HPP__

#define __STEPPER_HPP__

#include "cli.hpp"
#include "ctrls/ctrls.hpp"
#include "observer/observer.hpp"
#include "archive/archive.hpp"

#include "robots/foc/focmotor.hpp"
#include "hal/timer/pwm/gpio_pwm.hpp"


#ifdef STEPPER_NO_PRINT
#define CLI_PRINTS(...)
#define ARCHIVE_PRINTS(...)
#define CLI_DEBUG(...)
#define COMMAND_DEBUG(...)
#define RUN_DEBUG(...)

#else
#define CLI_PRINTS(...) logger.prints(__VA_ARGS__);
#define ARCHIVE_PRINTS(...) if(outen) logger.prints(__VA_ARGS__);

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

class FOCStepper:public StepperUtils::CliSTA, public FOCMotor{
    using StatLed = StepperComponents::StatLed;

    using Archive = StepperUtils::Archive;
    using Switches = StepperUtils::Switches;

    Archive archive_;
    Switches & switches_ = archive_.switches;
    volatile RunStatus run_status = RunStatus::INIT;
    volatile CtrlType ctrl_type = CtrlType::POSITION;

    SVPWM2 & svpwm;
    OdometerPoles odo;
    Memory & memory;

    GpioPwm red_pwm{portC[14]};
    GpioPwm green_pwm{portC[15]};
    GpioPwm blue_pwm{portC[13]};
    RgbLedAnalog rgb_led{red_pwm, green_pwm, blue_pwm};
    StatLed panel_led = StatLed{rgb_led, run_status, ctrl_type};

    real_t elecrad_zerofix;
    real_t run_elecrad;
    real_t est_elecrad;
    real_t run_leadangle;

    CtrlLimits ctrl_limits;

    CurrentCtrl::Config curr_config;
    CurrentCtrl curr_ctrl{curr_config};
    
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

    ErrorCode error_code = ErrorCode::OK;
    const char * error_message = nullptr;
    const char * warn_message = nullptr;

    bool shutdown_when_error_occurred = true;
    bool shutdown_when_warn_occurred = true;

    void throw_error(const ErrorCode _error_code,const char * _error_message) {
        error_message = _error_message;
        run_status = RunStatus::ERROR;
        if(shutdown_when_error_occurred){
            shutdown();
        }
        CLI_PRINTS(error_message);
    }

    void throw_warn(const ErrorCode ecode, const char * _warn_message){
        warn_message = _warn_message;
        run_status = RunStatus::WARN;
        if(shutdown_when_warn_occurred){
            shutdown();
        }
        CLI_PRINTS(warn_message);
    }

    #define THROW_ERROR(code, msg) throw_error(code,msg)
    #define THROW_WARN(code, msg) throw_warn(code,msg)

    RunStatus cali_task(const InitFlag init_flag = false);
    RunStatus active_task(const InitFlag init_flag = false);
    RunStatus beep_task(const InitFlag init_flag = false);
    RunStatus check_task(const InitFlag init_flag = false);


    void parseTokens(const String & _command, const std::vector<String> & args) override;

    void parseCommand(const Command command, const CanMsg & msg) override;

public:


    FOCStepper(IOStream & _logger, Can & _can, SVPWM2 & _svpwm, Encoder & encoder, Memory & _memory):
            CliSTA(_logger, _can, getNodeId()) ,svpwm(_svpwm), odo(encoder), memory(_memory){;}

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

    void setTargetCurrent(const real_t current){
        target = current;
        ctrl_type = CtrlType::CURRENT;
    }

    void setTargetSpeed(const real_t speed){
        target = speed;
        ctrl_type = CtrlType::SPEED;
    }

    void setTargetPosition(const real_t pos){
        target = pos;
        ctrl_type = CtrlType::POSITION;
    }

    void setTargetTrapezoid(const real_t pos){
        target = pos;
        ctrl_type = CtrlType::TRAPEZOID;
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
        curr_config.curr_limit = current;
    }

    void locateRelatively(const real_t pos = 0){
        odo.locateRelatively(pos);
    }

    void run();

    void report();

    bool isActive() const {
        return (RunStatus::ACTIVE) == run_status;
    }

    const volatile RunStatus & status(){
        return run_status;
    }


    real_t getSpeed() const{
        return measurements.spd;
    }

    real_t getPosition() const {
        return measurements.pos;
    }

    real_t getCurrent() const {
        return measurements.curr;
    }

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

    void setNodeId(const uint8_t _id){
        node_id = _id;
    }

    real_t getPositionErr(){
        return getPosition() - target;
    }

    real_t getSpeedErr(){
        return getSpeed() - target;
    }
    uint8_t getNodeId(){
        auto chip_id = Sys::Chip::getChipIdCrc();
        switch(chip_id){
            case 3273134334:
                return node_id = 3;
            case 341554774:
                return node_id = 2;
            case 4079188777:
                return node_id = 1;
            case 0:
            default:
                return node_id = 0;
        }
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