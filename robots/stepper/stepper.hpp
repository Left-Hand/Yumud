#ifndef __STEPPER_HPP__

#define __STEPPER_HPP__

#include "cli.hpp"
#include "ctrls.hpp"
#include "observer/observer.hpp"
#include "archive/archive.hpp"
#include "hal/adc/adcs/adc1.hpp"


class Stepper:public StepperUtils::Cli{
public:
    using ErrorCode = StepperEnums::ErrorCode;
    using RunStatus = StepperEnums::RunStatus;
protected:
    using ExitFlag = StepperEnums::ExitFlag;
    using InitFlag = StepperEnums::InitFlag;

    using Range = Range_t<real_t>;

    using CtrlType = StepperEnums::CtrlType;
    using Switches = StepperUtils::Switches;

    Switches switches;

    SVPWM2 & svpwm;

    OdometerPoles odo;
    Memory & memory;

    RgbLedDigital<true> led_instance{portC[14], portC[15], portC[13]};
    StatLed panel_led = StatLed{led_instance};

    real_t openloop_current = 0.8; 

    real_t est_speed;
    real_t raw_pos;
    real_t est_pos;
    real_t est_elecrad;
    real_t elecrad_zerofix;

    real_t run_current;
    real_t run_elecrad;
    real_t run_leadangle;

    real_t target;
    Range target_position_clamp = Range{std::numeric_limits<iq_t>::min(), std::numeric_limits<iq_t>::max()};

    uint8_t node_id = 0;

    CurrentCtrl curr_ctrl;
    GeneralSpeedCtrl speed_ctrl{curr_ctrl};
    GeneralPositionCtrl position_ctrl{curr_ctrl};
    TrapezoidPosCtrl trapezoid_ctrl{speed_ctrl, position_ctrl};

    volatile RunStatus run_status = RunStatus::INIT;
    CtrlType ctrl_type = CtrlType::POSITION;

    uint64_t exe_micros = 0;

    bool skip_tone = true;
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


    void parse_command(const String & _command, const std::vector<String> & args) override{
        auto command = _command;
        command.toLowerCase();
        switch(hash_impl(command.c_str(), command.length())){
            case "save"_ha:
            case "sv"_ha:
                saveArchive();
                break;

            case "load"_ha:
            case "ld"_ha:
                loadArchive();
                break;
            
            case "pos.p"_ha:
                settle_value(position_ctrl.kp, args);
                break;
            case "pos.d"_ha:
                settle_value(position_ctrl.kd, args);
                break;

            case "speed"_ha:
            case "spd"_ha:
            case "s"_ha:
                if(args.size()){
                    real_t spd = real_t(args[0]);
                    setTargetSpeed(spd);
                    logger << "targ speed\t" << toString(spd,2) << " n/s\r\n";
                }
                break;

            case "position"_ha:
            case "pos"_ha:
            case "p"_ha:
                if(args.size()){
                    real_t pos = real_t(args[0]);
                    setTargetPosition(pos);
                    logger << "targ position\t" << toString(pos,2) << " n\r\n";
                }
                break;

            case "eleczero"_ha:
            case "ez"_ha:
                settle_value(elecrad_zerofix, args);
                break;

            case "error"_ha:
            case "err"_ha:
                if(error_message) {DEBUG_PRINT(error_message)}
                else {DEBUG_PRINT("no error")}
                break;

            case "warn"_ha:
            case "wa"_ha:
                if(warn_message) {DEBUG_PRINT(warn_message)}
                else {DEBUG_PRINT("no warn")}
                break;

            case "enable"_ha:
            case "en"_ha:
            case "e"_ha:
                logger.println("enabled");
                wakeup();
                break;
            
            case "exe"_ha:
                logger << "exe" << exe_micros << "us\r\n";
                break;

            case "disable"_ha:
            case "dis"_ha:
            case "de"_ha:
            case "d"_ha:
                logger.println("disabled");
                shutdown();
                break;

            case "cali"_ha:
                cali_task(true);
                break;


            case "beep"_ha:
                beep_task(true);
                break;

            case "rd"_ha:
                if(args.size() == 1) run_debug_enabled = int(args[0]);
                break;
            case "status"_ha:
            case "stat"_ha:
                DEBUG_PRINT("current status:", int(run_status));
                break;

            case "shutdown"_ha:
            case "shut"_ha:
                shutdown();
                DEBUG_PRINT("shutdown ok");
                break;


            case "remove"_ha:
            case "rm"_ha:
                // shutdown();
                removeArchive();
                // DEBUG_PRINT("shutdown ok");
                break;

            default:
                Cli::parse_command(command, args);
                break;
        }
    }



    void parse_command(const Command command, const CanMsg & msg) override{
        const uint16_t tx_id = ((uint16_t)(node_id << 7 | (uint8_t)command));

        #define SET_METHOD_BIND_VOID(cmd, method)\
        case cmd:\
            method();\
            break;\

        #define SET_METHOD_BIND_EXECUTE(cmd, method, ...)\
        case cmd:\
            method(__VA_ARGS__);\
            break;\

        #define SET_METHOD_BIND(cmd, method, type)\
        case cmd:\
            method(type(msg));\
            break;\
        
        #define SET_VALUE_BIND(cmd, value)\
        case cmd:\
            value = (decltype(value)(msg));\
            break;\

        #define SET_METHOD_BIND_REAL(cmd, method) SET_METHOD_BIND(cmd, method, real_t)

        #define GET_VALUE(cmd, value)\
            case cmd:\
                if(msg.isRemote()){\
                    CanMsg msg {tx_id, false};\
                    can.write(msg.load(value));\
                }\
                break;\
        
        switch(command){

            SET_VALUE_BIND(Command::SET_TARGET, target)

            SET_METHOD_BIND_REAL(Command::TRG_VECT, setTargetVector)
            SET_METHOD_BIND_REAL(Command::TRG_CURR, setTargetCurrent)
            SET_METHOD_BIND_REAL(Command::TRG_POS, setTargetPosition)
            SET_METHOD_BIND_REAL(Command::TRG_TPZ, setTagretTrapezoid)

            SET_METHOD_BIND_REAL(Command::LOCATE, locateRelatively)
            SET_METHOD_BIND_REAL(Command::CLAMP_CURRENT, setCurrentClamp)
            SET_METHOD_BIND(Command::CLAMP_POS, setTargetPositionClamp, (std::tuple<real_t, real_t>))
            SET_METHOD_BIND(Command::CLAMP_SPD, setSpeedClamp, (real_t))
            SET_METHOD_BIND(Command::CLAMP_ACC, setAccelClamp, (real_t))

            GET_VALUE(Command::GET_POS, est_pos)
            GET_VALUE(Command::GET_SPD, est_speed)
            GET_VALUE(Command::GET_ACC, 0)


            SET_METHOD_BIND_EXECUTE(Command::INACTIVE, enable, false)
            SET_METHOD_BIND_EXECUTE(Command::ACTIVE, enable, true)
            SET_METHOD_BIND_EXECUTE(Command::SET_NODEID, setNodeId, msg.to<uint8_t>())
            default:
                break;
        }

        #undef SET_METHOD_BIND
        #undef SET_VALUE_BIND
        #undef SET_METHOD_BIND_REAL
        #undef SET_VALUE_BIND_REAL
    }
    
    friend ShutdownFlag;
public:
    void loadArchive();
    void saveArchive();
    void removeArchive();

    Stepper(IOStream & _logger, Can & _can, SVPWM2 & _svpwm, Encoder & encoder, Memory & _memory):
            Cli(_logger,_can) ,svpwm(_svpwm), odo(encoder), memory(_memory){;}




    void tick(){
        auto begin_micros = micros();
        RunStatus exe_status = RunStatus::NONE;

        switch(run_status){
            case RunStatus::INIT:
                {
                    // static bool load_lock = false;
                    // bool load_ok = autoload();
                    // // bool load_ok = false;
                    // if(load_ok){
                    //     run_status = RunStatus::CALI;
                    //     new_status = RunStatus::EXIT;
                    //     logger.println("autoload ok");
                    // }else{
                        check_task(true);
                    //     logger.println("autoload failed");
                    // }
                    break;
                }

            case RunStatus::CHECK:
                exe_status = check_task();
                break;

            case RunStatus::CALI:
                exe_status = cali_task();
                break;

            case RunStatus::ACTIVE:
                exe_status = active_task();
                break;

            case RunStatus::BEEP:
                exe_status = beep_task();
                break;

            case RunStatus::INACTIVE:
                run_status = RunStatus::ACTIVE;
                break;

            default:
                break;
        }

        //decide next status from execution result 

        if(not (exe_status == (RunStatus::NONE))){//execution meet sth.

            if((exe_status == RunStatus::ERROR)){
                // logger.println("exit");
                // logger.println(RunStatus.to_name());
                // logger.
                // shutdown_flag = true;
            }

            else if((exe_status == RunStatus::EXIT)){
                switch(run_status){
                    case RunStatus::CHECK:
                        panel_led.setTranstit(Color(), Color(0,0,1,0), StatLed::Method::Squ);
                        cali_task(true);
                        break;
                    case RunStatus::CALI:
                        if(skip_tone){
                            panel_led.setTranstit(Color(), Color(0,0,1,0), StatLed::Method::Squ);
                            active_task(true);
                        }
                        else beep_task(true);
                        break;
                    case RunStatus::BEEP:
                        active_task(true);
                        break;
                    case RunStatus::ACTIVE:
                        break;
                    case RunStatus::INACTIVE:
                        break;
                    case RunStatus::ERROR:
                        break;
                    case RunStatus::WARN:
                        break;
                    default:
                    break;
                }
            }else{
                switch(run_status){
                    case RunStatus::CHECK:
                        check_task(true);
                        break;
                    case RunStatus::CALI:
                        cali_task(true);
                        break;
                    case RunStatus::ACTIVE:
                        active_task(true);
                        break;
                    case RunStatus::BEEP:
                        beep_task(true);
                        break;
                    case RunStatus::INACTIVE:
                        break;
                    case RunStatus::ERROR:
                        break;
                    case RunStatus::WARN:
                        break;
                    default:
                        break;
                }
            }
        }
        exe_micros = micros() - begin_micros;
    }

    bool autoload();

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

    void report(){
        // real_t total = real_t(3);
        // static real_t freq = real_t(10);
        // static real_t freq_dir = real_t(1);
        // const real_t freq_delta = real_t(20);
        // if(freq > real_t(300)) freq_dir = real_t(-1);
        // else if(freq < real_t(4)) freq_dir = real_t(1);
        // static real_t last_t = t;
        // real_t delta = (t - last_t);
        // freq += delta * freq_dir * freq_delta;
        // last_t = t;
        // static real_t ang = real_t(0);
        // ang += freq * delta;
        // real_t target = (total / freq) * sin(ang);

        // target_pos = sign(frac(t) - 0.5);
        // target_pos = sin(t);
        // RUN_DEBUG(, est_pos, est_speed);
        if(run_status == RunStatus::ACTIVE and logger.pending() == 0) RUN_DEBUG(target, est_speed, est_pos, run_current, run_leadangle);
        // delay(1);
        // , est_speed, t, odo.getElecRad(), openloop_elecrad);
        // logger << est_pos << est_speed << run_current << elecrad_zerofix << endl;
        // RUN_DEBUG(est_pos, est_speed, run_current, elecrad_zerofix);
        // RUN_DEBUG(est_pos, est_speed, run_current, run_elecrad);

        // bool led_status = (millis() / 200) % 2;
        // bled = led_status;
    }

    bool isActive() const {
        return (RunStatus::ACTIVE) == run_status;
    }

    const auto & status() const {
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
};

#endif