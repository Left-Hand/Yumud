#ifndef __STEPPER_HPP__

#define __STEPPER_HPP__

#include "cli.hpp"
#include "ctrls/ctrls.hpp"
#include "observer/observer.hpp"
#include "archive/archive.hpp"
#include "../hal/adc/adcs/adc1.hpp"
#include "../robots/stepper/concept.hpp"

class Stepper:public StepperUtils::CliSTA, public StepperConcept{

    using Archive = StepperUtils::Archive;
    using Switches = StepperUtils::Switches;

    Archive m_archive;
    Switches & m_switches = m_archive.switches;

    volatile RunStatus run_status = RunStatus::INIT;
    SVPWM2 & svpwm;

    OdometerPoles odo;
    Memory & memory;

    RgbLedDigital<true> led_instance{portC[14], portC[15], portC[13]};
    StatLed panel_led = StatLed{led_instance};

    real_t raw_pos;
    real_t est_elecrad;
    real_t elecrad_zerofix;

    real_t run_elecrad;
    real_t run_leadangle;

    Range target_position_clamp = Range::INF;

    CurrentCtrl::Config curr_config;
    CurrentCtrl curr_ctrl{curr_config};
    
    GeneralSpeedCtrl::Config spd_config;
    GeneralSpeedCtrl speed_ctrl{curr_ctrl, spd_config};
    
    GeneralPositionCtrl::Config pos_config;
    GeneralPositionCtrl position_ctrl{curr_ctrl, pos_config};

    TrapezoidPosCtrl::Config tpz_config;
    TrapezoidPosCtrl trapezoid_ctrl{speed_ctrl, position_ctrl, tpz_config};

    SpeedEstimator::Config spe_config;
    SpeedEstimator speed_estmator{spe_config};

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


    ErrorCode error_code = ErrorCode::OK;
    const char * error_message = nullptr;
    const char * warn_message = nullptr;

    bool shutdown_when_error_occurred = true;
    bool shutdown_when_warn_occurred = true;


    void setCurrent(const real_t _current, const real_t _elecrad){
        svpwm.setCurrent(_current, _elecrad);
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


    void parseTokens(const String & _command, const std::vector<String> & args) override;

    void parseCommand(const Command command, const CanMsg & msg) override;
    
    friend ShutdownFlag;


public:


    Stepper(IOStream & _logger, Can & _can, SVPWM2 & _svpwm, Encoder & encoder, Memory & _memory):
            CliSTA(_logger, _can, getNodeId()) ,svpwm(_svpwm), odo(encoder), memory(_memory){;}

    bool loadArchive(const bool outen = false);
    void saveArchive(const bool outen = false);
    void removeArchive(const bool outen = false);

    void setNozzle(const real_t duty);
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

    void setTargetTrapezoid(const real_t pos){
        target = pos;
        panel_led.setTranstit(Color(), Color(0,1,0,0), StatLed::Method::Squ);
        ctrl_type = CtrlType::TRAPEZOID;
    }

    void setOpenLoopCurrent(const real_t current){
        curr_config.current_clamp = current;
    }

    void setTargetVector(const real_t pos){
        target = pos;
        ctrl_type = CtrlType::VECTOR;
    }

    void freeze(){
        setTargetPosition(getPosition());
    }

    void setCurrentClamp(const real_t max_current){
        curr_config.current_clamp = max_current;
    }

    void locateRelatively(const real_t pos = 0){
        odo.locateRelatively(pos);
    }

    void run(){
        readCan();
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

    void setPositionClamp(const Range & clamp){
        target_position_clamp = clamp;
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

    uint8_t getNodeId(){
        // return 0;
        auto chip_id = Sys::Chip::getChipIdCrc();
        // logger.println("chip_id:", chip_id);
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

    void setSpeedClamp(const real_t max_spd){
        spd_config.max_spd = max_spd;
    }

    void setAccelClamp(const real_t max_acc){
        tpz_config.max_dec = max_acc;
    }

    void triggerCali(){
        cali_task(true);
    }

    void clear(){

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