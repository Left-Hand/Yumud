#ifndef __STEPPER_HPP__

#define __STEPPER_HPP__

#include "cli.hpp"
#include "ctrls.hpp"
#include "obs.hpp"
#include "archive.hpp"


using AT8222 = TB67H450;

class Stepper:public StepperUtils::Cli{
protected:
    using ExitFlag = StepperEnums::ExitFlag;
    using InitFlag = StepperEnums::InitFlag;

    using Range = Range_t<real_t>;
    using ErrorCode = StepperEnums::ErrorCode;
    using RunStatus = StepperEnums::RunStatus;
    using CtrlType = StepperEnums::CtrlType;
    using Switches = StepperUtils::Switches;

    Switches switches;
    IOStream & logger = uart1;

    AT8222 coilA{timer1.oc(3), timer1.oc(4), timer3.oc(3)};
    AT8222 coilB{timer1.oc(1), timer1.oc(2), timer3.oc(2)};


    SVPWM2 svpwm{coilA, coilB};

    SpiDrv mt6816_drv{spi1, 0};
    MT6816 mt6816{mt6816_drv};

    OdometerPoles odo = OdometerPoles(mt6816);

    I2cSw i2cSw{portD[1], portD[0]};
    AT24C02 at24{i2cSw};
    Memory memory{at24};

    uint32_t foc_pulse_micros;
    real_t est_speed;
    real_t raw_pos;
    real_t est_pos;
    real_t est_elecrad;

    real_t run_current;
    real_t run_elecrad;
    real_t run_leadangle;
    real_t elecrad_zerofix;

    real_t target;

    real_t openloop_elecrad;
    CurrentCtrl curr_ctrl;
    GeneralSpeedCtrl speed_ctrl{curr_ctrl};
    GeneralPositionCtrl position_ctrl{curr_ctrl};
    TrapezoidPosCtrl trapezoid_ctrl{speed_ctrl, position_ctrl};

    RunStatus run_status = RunStatus::INIT;
    CtrlType ctrl_type = CtrlType::POSITION;

    uint64_t exe_micros = 0;

    bool skip_tone = true;
    bool cmd_mode = false;


    void setCurrent(const real_t _current, const real_t _elecrad){
        svpwm.setCurrent(_current, _elecrad);
    }


    ErrorCode error_code = ErrorCode::OK;
    const char * error_message = nullptr;
    const char * warn_message = nullptr;

    bool auto_shutdown_activation = true;
    bool auto_shutdown_actived = false;

    uint16_t auto_shutdown_timeout_ms = 200;
    uint16_t auto_shutdown_last_wake_ms = 0;

    bool shutdown_when_error_occurred;
    bool shutdown_when_warn_occurred;

    void shutdown(){
        coilA.enable(false);
        coilB.enable(false);
    }

    void wakeup(){
        coilA.enable(true);
        coilB.enable(true);
    }

    struct ShutdownFlag{
    protected:
        bool state = false;
    public:

        ShutdownFlag() = default;

        auto & operator = (const bool _state){
            state = _state;

            //TODO
            // if(state) shutdown();
            // else wakeup();

            return *this;
        }

        operator bool() const{
            return state;
        }
    };


    ShutdownFlag shutdown_flag;



    void throw_error(const ErrorCode & _error_code,const char * _error_message) {
        error_message = _error_message;
        run_status = RunStatus::ERROR;
        if(shutdown_when_error_occurred){
            shutdown_flag = true;
        }
    }

    void throw_warn(const ErrorCode & ecode, const char * _warn_message){
        warn_message = _warn_message;
        run_status = RunStatus::WARN;
        if(shutdown_when_warn_occurred){
            shutdown_flag = true;
        }
    }


    RunStatus cali_task(const InitFlag init_flag = false);
    RunStatus active_task(const InitFlag init_flag = false);
    RunStatus beep_task(const InitFlag init_flag = false);
    RunStatus check_task(const InitFlag init_flag = false);

    RgbLedDigital<true> led_instance{portC[14], portC[15], portC[13]};
    StatLed panel_led = StatLed{led_instance};



    void parse_command(const String & _command, const std::vector<String> & args) override{
        auto command = _command;
        command.toLowerCase();
        switch(hash_impl(command.c_str(), command.length())){
            case "save"_ha:
            case "sv"_ha:
                saveArchive();
                break;

            // case ""
            case "load"_ha:
            case "ld"_ha:
                loadArchive();
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
                DEBUG_PRINT("current status:", run_status._to_string());
                break;

            case "shutdown"_ha:
            case "shut"_ha:
                shutdown();
                DEBUG_PRINT("shutdown ok");
                break;

            default:
                Cli::parse_command(command, args);
                break;
        }
    }
    
public:
    void loadArchive();
    void saveArchive();

    void tick(){

        auto begin_micros = micros();
        RunStatus new_status = RunStatus::NONE;

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
                        cali_task(true);
                    //     logger.println("autoload failed");
                    // }
                    break;
                }
            case RunStatus::CALI:
                new_status = cali_task();
                break;

            case RunStatus::ACTIVE:
                new_status = active_task();
                break;

            case RunStatus::BEEP:
                new_status = beep_task();
                break;

            case RunStatus::INACTIVE:
                run_status = RunStatus::ACTIVE;
                break;

            default:
                break;
        }

        if(not (+new_status == (+RunStatus::NONE))){
            if((+new_status == +RunStatus::EXIT)){
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
        using TimerUtils::Mode;
        using TimerUtils::IT;
        
        logger.setEps(4);

        timer1.init(chopper_freq, Mode::CenterAlignedDownTrig);
        timer1.enableArrSync();
        timer3.init(foc_freq, Mode::CenterAlignedDownTrig);
        timer3.enableArrSync();

        coilA.init();
        coilB.init();

        // timer3.oc(2).init()
        // setMode(TimerOC::Mode::Inactive);
        // timer3.oc(3).setMode(TimerOC::Mode::Inactive);
        // timer3.init();

        spi1.init(18000000);
        spi1.bindCsPin(portA[15], 0);

        i2cSw.init(400000);

        odo.init();

        panel_led.init();

        timer3.enableIt(IT::Update, NvicPriority(0, 0));
        timer3.bindCb(IT::Update, [&](){this->tick();});

        panel_led.setPeriod(400);
        panel_led.setTranstit(Color(), Color(1,0,0,0), StatLed::Method::Squ);
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

    void setTagretVector(const real_t pos){
        target = pos;
        ctrl_type = CtrlType::VECTOR;
    }

    void setCurrentClamp(const real_t max_current){
        curr_ctrl.setCurrentClamp(max_current);
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
        if(+run_status == +RunStatus::ACTIVE and logger.pending() == 0) RUN_DEBUG(target, est_speed, est_pos, run_current, run_leadangle);
        // delay(1);
        // , est_speed, t, odo.getElecRad(), openloop_elecrad);
        // logger << est_pos << est_speed << run_current << elecrad_zerofix << endl;
        // RUN_DEBUG(est_pos, est_speed, run_current, elecrad_zerofix);
        // RUN_DEBUG(est_pos, est_speed, run_current, run_elecrad);

        // bool led_status = (millis() / 200) % 2;
        // bled = led_status;
    }


};

#endif