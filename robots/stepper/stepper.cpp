#include "stepper.hpp"
#include "types/float/bf16.hpp"

static auto & nozzle_en_gpio = portA[0];

void Stepper::setNozzle(const real_t duty){
    nozzle_en_gpio.outpp();
    nozzle_en_gpio = bool(duty);
}

void Stepper::parseTokens(const String & _command, const std::vector<String> & args){
    auto command = _command;
    switch(hash_impl(command.c_str(), command.length())){
        case "save"_ha:
        case "sv"_ha:
            saveArchive(args.size() ? bool(int(args[0])) : false);
            break;

        case "load"_ha:
        case "ld"_ha:
            loadArchive(args.size() ? bool(int(args[0])) : true);
            break;

        case "nz"_ha:
            {
                real_t val = args.size() ? int(args[0]) : 0;
                setNozzle(val);
                CLI_PRINTS("set nozzle to:",val);
            }
            break;

        case "remove"_ha:
        case "rm"_ha:
            removeArchive(args.size() ? bool(int(args[0])) : true);
            break;

        case "speed"_ha:
        case "spd"_ha:
        case "s"_ha:
            if(args.size()){
                real_t spd = real_t(args[0]);
                setTargetSpeed(spd);
                CLI_PRINTS("targ speed\t", spd, " n/s");
            }else{
                CLI_PRINTS("curr speed\t", getSpeed(), " n/s");
            }
            break;

        case "position"_ha:
        case "pos"_ha:
        case "p"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                setTargetPosition(val);
                CLI_PRINTS("targ position\t", val, " n");
            }else{
                CLI_PRINTS("now position", getPosition(), " n");
            }
            break;

        case "tpz"_ha:
        case "t"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                setTargetTrapezoid(val);
                CLI_PRINTS("targ position\t", val, " n");
            }else{
                CLI_PRINTS("now position\t", getPosition(), " n");
            }
            break;

        case "stable"_ha:
            CLI_PRINTS(odo.encoder.stable());
            break;

        case "curr"_ha:
        case "c"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                setTargetCurrent(val);
                CLI_PRINTS("targ current\t", val, " n");
            }else{
                CLI_PRINTS("now current\t", getCurrent(), " n");
            }
            break;

        case "vect"_ha:
        case "v"_ha:
            if(args.size()){
                auto v = real_t(args[0]);
                setTargetVector(v);
                CLI_PRINTS("targ vector\t", v, " n");
            }
            break;

        case "crc"_ha:
            CLI_PRINTS(Sys::Chip::getChipIdCrc());
            break;

        case "eleczero"_ha:
        case "ez"_ha:{
            if(args.size() == 0) break;
            elecrad_zerofix = real_t(args[0]);
        }
            break;

        case "error"_ha:
        case "err"_ha:
            if(error_message) {CLI_PRINTS(error_message)}
            else {CLI_PRINTS("no error")}
            break;

        case "warn"_ha:
        case "wa"_ha:
            if(warn_message) {CLI_PRINTS(warn_message)}
            else {CLI_PRINTS("no warn")}
            break;

        case "enable"_ha:
        case "en"_ha:
        case "e"_ha:
            rework();
            CLI_PRINTS("enabled");
            break;
        
        case "exe"_ha:
            CLI_PRINTS("exe", exe_micros, "us");
            break;

        case "disable"_ha:
        case "dis"_ha:
        case "de"_ha:
        case "d"_ha:
            shutdown();
            CLI_PRINTS("disabled");
            break;

        case "cali"_ha:
            cali_task(true);
            CLI_PRINTS("cali started");
            break;

        case "locate"_ha:
        case "loc"_ha:
        {
            real_t loc = args.size() ? real_t(args[0]) : 0;
            locateRelatively(loc);
            CLI_PRINTS("located to", loc);
        }
            break;

        case "beep"_ha:
            beep_task(true);
            break;

        case "id"_ha:
            CLI_PRINTS("node id is: ", node_id);
            break;

        case "rd"_ha:
            run_debug_enabled = args.size() ? int(args[0]) : true;
            CLI_PRINTS("run debug enabled:", run_debug_enabled);
            break;

        case "cl"_ha:{
            auto cl = args.size() ? real_t(args[0]) : 0;
            CLI_PRINTS("current clamp:", cl);
        }
            break;

        case "status"_ha:
        case "stat"_ha:
            CLI_PRINTS("current status:", int(run_status));
            break;

        case "shutdown"_ha:
        case "shut"_ha:
            shutdown();
            CLI_PRINTS("shutdown ok");
            break;

        case "cd"_ha:
            CLI_PRINTS("dir changed");
            elecrad_zerofix = PI;
            break;

        case "hlt"_ha:
            CLI_PRINTS("halt");
            CREATE_FAULT;
            break;

        case "map"_ha:
            for(const auto & item : odo.map()){
                CLI_PRINTS(item);
                delay(1);
            }
            break;

        case "version"_ha:
        case "ver"_ha:
            break;
        case "info"_ha:
            CLI_PRINTS(archive_.board_info);
            break;
        default:
            CliSTA::parseTokens(command, args);
            break;
    }
}


void Stepper::parseCommand(const Command command, const CanMsg & msg){
    const uint16_t tx_id = (((uint16_t)(node_id) << 7) | (uint8_t)(command));

    using E = bf16;
    using E_2 = std::tuple<E, E>;

    #define SET_METHOD_BIND_EXECUTE(cmd, method, ...)\
    case cmd:\
        method(__VA_ARGS__);\
        break;\

    #define SET_METHOD_BIND_TYPE(cmd, method, type)\
    case cmd:\
        method(type(msg));\
        break;\
    
    #define SET_VALUE_BIND(cmd, value)\
    case cmd:\
        value = (decltype(value)(msg));\
        break;\

    #define SET_METHOD_BIND_ONE(cmd, method) SET_METHOD_BIND_TYPE(cmd, method, E)

    #define GET_BIND_VALUE(cmd, value)\
        case cmd:\
            if(msg.isRemote()){\
                can.write(CanMsg(tx_id, value));\
            }\
            break;\
    
    switch(command){

        SET_VALUE_BIND(Command::SET_TARGET, target)

        SET_METHOD_BIND_ONE(   Command::SET_TRG_VECT,  setTargetVector)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_CURR,  setTargetCurrent)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_POS,   setTargetPosition)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_SPD,   setTargetSpeed)
        SET_METHOD_BIND_ONE(   Command::SET_TRG_TPZ,   setTargetTrapezoid)
        SET_METHOD_BIND_EXECUTE(Command::FREEZE,        freeze)

        SET_METHOD_BIND_ONE(   Command::LOCATE,        locateRelatively)
        SET_METHOD_BIND_ONE(   Command::SET_OPEN_CURR, setOpenLoopCurrent)
        SET_METHOD_BIND_ONE(   Command::SET_CURR_LMT,  setCurrentClamp)
        SET_METHOD_BIND_TYPE(   Command::SET_POS_LMT,   setPositionClamp, E_2)
        SET_METHOD_BIND_ONE(   Command::SET_SPD_LMT,   setSpeedClamp)
        SET_METHOD_BIND_ONE(   Command::SET_ACC_LMT,   setAccelClamp)

        GET_BIND_VALUE(         Command::GET_POS,       measurements.pos)
        GET_BIND_VALUE(         Command::GET_SPD,       measurements.spd)
        GET_BIND_VALUE(         Command::GET_ACC,       0)//TODO

        SET_METHOD_BIND_EXECUTE(Command::TRG_CALI,          triggerCali)

        SET_METHOD_BIND_EXECUTE(Command::SAVE,          saveArchive)
        SET_METHOD_BIND_EXECUTE(Command::LOAD,          loadArchive)
        SET_METHOD_BIND_EXECUTE(Command::CLEAR,         removeArchive)

        SET_METHOD_BIND_EXECUTE(Command::NOZZLE_ON,     setNozzle, 1)
        SET_METHOD_BIND_EXECUTE(Command::NOZZLE_OFF,    setNozzle, 0)

        SET_METHOD_BIND_EXECUTE(Command::RST,           reset)
        GET_BIND_VALUE(         Command::STAT,          (uint8_t)run_status);
        SET_METHOD_BIND_EXECUTE(Command::INACTIVE,      enable, false)
        SET_METHOD_BIND_EXECUTE(Command::ACTIVE,        enable, true)
        SET_METHOD_BIND_EXECUTE(Command::SET_NODEID,    setNodeId, uint8_t(msg))

        default:
            CliSTA::parseCommand(command, msg);
            break;
    }

    #undef SET_METHOD_BIND
    #undef SET_VALUE_BIND
    #undef SET_METHOD_BIND_ONE
    #undef SET_VALUE_BIND_REAL
}


void Stepper::tick(){
    auto begin_micros = micros();
    RunStatus exe_status = RunStatus::NONE;

    switch(run_status){
        case RunStatus::INIT:
            {
                bool load_ok = loadArchive(false);
                if(load_ok){
                    if(skip_tone){
                        panel_led.setTranstit(Color(), Color(0,0,1,0), StatLed::Method::Squ);
                        active_task(true);
                    }else{
                        beep_task(true);
                    }
                }else{
                    cali_task(true);
                }
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

    //decide next status by execution result 

    if(not (exe_status == (RunStatus::NONE))){//execution meet sth.

        if((exe_status == RunStatus::ERROR)){
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

void Stepper::run(){
    readCan();
    panel_led.run();


    #ifndef STEPPER_NO_PRINT
    {
        static String temp;
        while(logger.available()){
            auto chr = logger.read();
            if(chr == 0) continue;
            temp += chr;
            if(chr == '\n'){
                temp.alphanum();
                parseLine(temp);
                temp = "";
            }
        }
    }
    #endif

    red_pwm.tick();
    green_pwm.tick();
    blue_pwm.tick();
}

void Stepper::report(){
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
    if(logger.pending()==0){
        // delayMicroseconds(200);   
        // delay(1); 
        RUN_DEBUG(std::setprecision(4), target, getSpeed(), getPosition(), getCurrent(), run_leadangle,std::setprecision(4), getPositionErr());
    }
    // delay(1);
    // , est_speed, t, odo.getElecRad(), openloop_elecrad);
    // logger << est_pos << est_speed << run_current << elecrad_zerofix << endl;
    // RUN_DEBUG(est_pos, est_speed, run_current, elecrad_zerofix);
    // RUN_DEBUG(est_pos, est_speed, run_current, run_elecrad);

    // bool led_status = (millis() / 200) % 2;
    // bled = led_status;
}