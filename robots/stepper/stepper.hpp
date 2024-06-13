#ifndef __STEPPER_HPP__

#define __STEPPER_HPP__

#include "cli.hpp"
#include "ctrls.hpp"
#include "obs.hpp"
#include "archive.hpp"

class Stepper:public StepperUtils::Cli{
protected:
    using ExitFlag = bool;
    using InitFlag = bool;

    using Range = Range_t<real_t>;
    using ErrorCode = StepperEnums::ErrorCode;
    using RunStatus = StepperEnums::RunStatus;
    using CtrlType = StepperEnums::CtrlType;

    IOStream & logger = uart1;

    TimerOutChannelPosOnChip & verfChannelA = timer3[3];
    TimerOutChannelPosOnChip & verfChannelB = timer3[2];

    Coil1 coilA{portA[10], portA[11],  verfChannelA};
    Coil1 coilB{portA[8], portA[9],  verfChannelB};

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
    real_t run_raddiff;
    real_t elecrad_zerofix;

    real_t target;

    real_t openloop_elecrad;
    CurrentCtrl curr_ctrl;
    GeneralSpeedCtrl speed_ctrl{curr_ctrl};
    GeneralPositionCtrl position_ctrl{curr_ctrl};
    RunStatus run_status = RunStatus::INIT;
    CtrlType ctrl_type = CtrlType::POSITION;

    bool skip_tone = true;
    bool cmd_mode = false;


    void setCurrent(const real_t & _current, const real_t & _elecrad){
        coilA = cos(_elecrad) * _current;
        coilB = sin(_elecrad) * _current;
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

        auto & operator = (const bool & _state){
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
                saveAchive();
                break;

            case "load"_ha:
            case "ld"_ha:
                loadAchive();
                break;

            case "speed"_ha:
            case "spd"_ha:
            case "s"_ha:
                if(args.size()) setTargetSpeed(real_t(args[0]));
                break;

            case "position"_ha:
            case "pos"_ha:
            case "p"_ha:
                if(args.size()) setTargetPosition(real_t(args[0]));
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
            
            case "disable"_ha:
            case "dis"_ha:
            case "de"_ha:
            case "d"_ha:
                logger.println("disabled");
                shutdown();
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
    

    void saveAchive(){
        using Archive = StepperUtils::Archive;
        Archive archive;
        memory.store(archive);
    }

    void loadAchive(){
        using Archive = StepperUtils::Archive;
        Archive archive;
        memory.load(archive);

        logger << "build version:\t\t" << archive.bver << "\r\n";
        logger << "build time:\t\t20" << 
                archive.y << '/' << archive.m << '/' << 
                archive.d << '\t' << archive.h << ':' << archive.mi << "\r\n";

        logger << "driver type:\t\t" << archive.dtype << "\r\n";
        logger << "driver branch:\t\t" << archive.dbranch << "\r\n";
    }
public:

    void tick(){
        RunStatus new_status = RunStatus::NONE;

        switch(run_status){
            case RunStatus::INIT:
                cali_task(true);
                break;
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
                        cali_task(true);
                        break;
                    case RunStatus::CALI:
                        if(skip_tone) active_task(true);
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

    }

    void init(){

        uart1.init(115200 * 8);
        // uart1.init(115200 * 8);

        logger.setEps(4);

        timer1.init(4096, 1, Timer::TimerMode::CenterAlignedDownTrig);
        timer1.enableArrSync();

        timer3.init(1024, 1, Timer::TimerMode::CenterAlignedDownTrig);
        timer3.enableArrSync();

        svpwm.init();

        coilA.setClamp(real_t(1));
        coilB.setClamp(real_t(1));

        coilA.init();
        coilB.init();

        spi1.init(18000000);
        spi1.bindCsPin(portA[15], 0);

        logger.println("pwon");
        odo.init();

        // adc1.init(
        //     {},{
        //         AdcChannelConfig{.channel = AdcChannels::CH3, .sample_cycles = AdcSampleCycles::T71_5},
        //         AdcChannelConfig{.channel = AdcChannels::CH4, .sample_cycles = AdcSampleCycles::T71_5}
        //     });
        // adc1.setRegularTrigger(AdcOnChip::RegularTrigger::SW);
        // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::SW);
        // timer3[4] = 0;
        // adc1.setPga(AdcOnChip::Pga::X64);
        // adc1.setInjectedTrigger(AdcOnChip::InjectedTrigger::T3CC4);
        // TIM3->CH4CVR = TIM3->ATRLR >> 1;
        // adc1.enableCont();
        // adc1.enableScan();
        // adc1.enableAutoInject();

        // auto & bled = portC[13];
        // bled.OutPP();
        panel_led.init();

        timer4.init(foc_freq);
        timer4.enableIt(Timer::IT::Update, NvicPriority(0, 0));
        timer4.bindCb(Timer::IT::Update, [&](){this->tick();});


        panel_led.setPeriod(200);
        panel_led.setTranstit(Color(), Color(0,1,0,0), StatLed::Method::Squ);
    }

    void setTargetSpeed(const real_t speed){
        target = speed;
        ctrl_type = CtrlType::SPEED;
    }

    void setTargetPosition(const real_t speed){
        target = speed;
        ctrl_type = CtrlType::POSITION;
    }

    void run() override{
        Cli::run();
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
        // RUN_DEBUG(odo.getPosition(), est_pos, est_speed, ctrl.elecrad_offset_output, odo.getRawLapPosition(), odo.getLapPosition());
        // if(DEBUGGER.pending() == 0) RUN_DEBUG(target, est_speed, est_pos, run_current, run_raddiff);
        // , est_speed, t, odo.getElecRad(), openloop_elecrad);
        // logger << est_pos << est_speed << run_current << elecrad_zerofix << endl;
        // RUN_DEBUG(est_pos, est_speed, run_current, elecrad_zerofix);
        // RUN_DEBUG(est_pos, est_speed, run_current, run_elecrad);
        static String temp_str = "";

        // bool led_status = (millis() / 200) % 2;
        // bled = led_status;
        panel_led.run();


        Sys::Clock::reCalculateTime();
    }


};

#endif