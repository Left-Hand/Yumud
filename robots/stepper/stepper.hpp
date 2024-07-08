#ifndef __STEPPER_HPP__

#define __STEPPER_HPP__

#include "cli.hpp"
#include "ctrls.hpp"
#include "observer/observer.hpp"
#include "archive/archive.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "robots/stepper/concept.hpp"

class Stepper:public StepperUtils::CliSTA, public StepperConcept{
    volatile RunStatus run_status = RunStatus::INIT;
    Switches switches;

    SVPWM2 & svpwm;

    OdometerPoles odo;
    Memory & memory;

    RgbLedDigital<true> led_instance{portC[14], portC[15], portC[13]};
    StatLed panel_led = StatLed{led_instance};

    real_t openloop_current = 0.8; 

    real_t raw_pos;
    real_t est_elecrad;
    real_t elecrad_zerofix;

    real_t run_current;
    real_t run_elecrad;
    real_t run_leadangle;

    Range target_position_clamp = Range{std::numeric_limits<iq_t>::min(), std::numeric_limits<iq_t>::max()};

    CurrentCtrl curr_ctrl;
    GeneralSpeedCtrl speed_ctrl{curr_ctrl};
    GeneralPositionCtrl position_ctrl{curr_ctrl};
    TrapezoidPosCtrl trapezoid_ctrl{speed_ctrl, position_ctrl};

    bool cali_debug_enabled = true;
    bool command_debug_enabled = false;
    bool run_debug_enabled = false;

    #define CALI_DEBUG(...)\
    if(cali_debug_enabled){\
    logger.println(__VA_ARGS__);};

    #define COMMAND_DEBUG(...)\
    if(command_debug_enabled){\
    logger.println(__VA_ARGS__);};

    #define RUN_DEBUG(...)\
    if(run_debug_enabled){\
    logger.println(__VA_ARGS__);};
    
    CtrlType ctrl_type = CtrlType::POSITION;

    uint64_t exe_micros = 0;

    bool skip_tone = false;
    bool cmd_mode = false;

    ShutdownFlag shutdown_flag{*this};

    void setCurrent(const real_t _current, const real_t _elecrad){
        svpwm.setCurrent(_current, _elecrad);
    }

    ErrorCode error_code = ErrorCode::OK;
    String error_message;
    String warn_message;

    bool shutdown_when_error_occurred;
    bool shutdown_when_warn_occurred;

    void shutdown(){
        svpwm.enable(false);
    }

    void wakeup(){
        svpwm.enable(true);
    }

    bool on_exeception(){
        return RunStatus::WARN == run_status || RunStatus::ERROR == run_status; 
    }

    void throw_error(const ErrorCode & _error_code,const char * _error_message) {
        error_message = _error_message;
        run_status = RunStatus::ERROR;
        if(shutdown_when_error_occurred){
            shutdown_flag = true;
        }
        logger.println(error_message);
    }

    void throw_warn(const ErrorCode & ecode, const char * _warn_message){
        warn_message = _warn_message;
        run_status = RunStatus::WARN;
        if(shutdown_when_warn_occurred){
            shutdown_flag = true;
        }
        logger.println(warn_message);
    }


    RunStatus cali_task(const InitFlag init_flag = false);
    RunStatus active_task(const InitFlag init_flag = false);
    RunStatus beep_task(const InitFlag init_flag = false);
    RunStatus check_task(const InitFlag init_flag = false);


    void parse_command(const String & _command, const std::vector<String> & args) override;

    void parse_command(const Command command, const CanMsg & msg) override;
    
    friend ShutdownFlag;

    uint8_t getNodeID(){
        return 0;
    }
public:


    Stepper(IOStream & _logger, Can & _can, SVPWM2 & _svpwm, Encoder & encoder, Memory & _memory):
            CliSTA(_logger, _can, getNodeID()) ,svpwm(_svpwm), odo(encoder), memory(_memory){;}

    bool loadArchive(const bool outen);
    void saveArchive(const bool outen);
    void removeArchive(const bool outen);
    bool autoload(const bool outen);

    void tick();


    void init(){
        odo.init();

        panel_led.init();
        panel_led.setPeriod(400);
        panel_led.setTranstit(Color(), Color(1,0,0,0), StatLed::Method::Squ);
    }

    void setTargetCurrent(const real_t current){
        target = current;
        panel_led.setTranstit(Color(), Color(0,1,0,0), StatLed::Method::Squ);
        ctrl_type = CtrlType::CURRENT;
    }

    void setTargetSpeed(const real_t speed){
        target = speed;
        panel_led.setTranstit(Color(), Color(0,1,0,0), StatLed::Method::Squ);
        ctrl_type = CtrlType::SPEED;
    }

    void setTargetPosition(const real_t pos){
        target = pos;
        panel_led.setTranstit(Color(), Color(0,1,0,0), StatLed::Method::Squ);
        ctrl_type = CtrlType::POSITION;
    }

    void setTagretTrapezoid(const real_t pos){
        target = pos;
        panel_led.setTranstit(Color(), Color(0,1,0,0), StatLed::Method::Squ);
        ctrl_type = CtrlType::TRAPEZOID;
    }

    void setOpenLoopCurrent(const real_t current){
        openloop_current = current;
    }

    void setTargetVector(const real_t pos){
        target = pos;
        ctrl_type = CtrlType::VECTOR;
    }

    void setCurrentClamp(const real_t max_current){
        curr_ctrl.setCurrentClamp(max_current);
    }

    void locateRelatively(const real_t pos = 0){
        odo.locateRelatively(pos);
    }

    void run() override{
        Cli::run();
        panel_led.run();
    }

    void report();

    bool isActive() const {
        return (RunStatus::ACTIVE) == run_status;
    }

    const volatile RunStatus & status(){
        return run_status;
    }


    real_t getSpeed() const{
        return est_speed;
    }

    real_t getPosition() const {
        return est_pos;
    }

    real_t getCurrent() const {
        return run_current;
    }

    void setTargetPositionClamp(const Range & clamp){
        target_position_clamp = clamp;
    }

    void enable(const bool en = true){
        if(en){
            wakeup();
        }else{
            shutdown();
        }
    }

    void setNodeId(const uint8_t _id){
        node_id = _id;
    }

    void setSpeedClamp(const real_t max_spd){
        speed_ctrl.max_spd = max_spd;
    }

    void setAccelClamp(const real_t max_acc){
        trapezoid_ctrl.max_dec = max_acc;
    }

    void triggerCali(){
        cali_task(true);
    }
};

#endif