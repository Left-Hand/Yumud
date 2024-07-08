#include "stepper.hpp"

void Stepper::parse_command(const String & _command, const std::vector<String> & args){
    auto command = _command;
    command.toLowerCase();
    switch(hash_impl(command.c_str(), command.length())){
        case "save"_ha:
        case "sv"_ha:
            saveArchive(true);
            break;

        case "load"_ha:
        case "ld"_ha:
            {
                bool outen = true;

                if(args.size()){
                    outen &= bool(int(args[0]));
                }
                
                loadArchive(outen);
            }
            break;

        case "remove"_ha:
        case "rm"_ha:
            {
                bool outen = true;

                if(args.size()){
                    outen &= bool(int(args[0]));
                }
                
                removeArchive(outen);
            }
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

        case "autoload"_ha:
        case "ald"_ha:
            {
                bool outen = true;

                if(args.size()){
                    outen &= bool(int(args[0]));
                }

                autoload(outen);
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




        default:
            Cli::parse_command(command, args);
            break;
    }
}


void Stepper::parse_command(const Command command, const CanMsg & msg){
    const uint16_t tx_id = ((uint16_t)(node_id << 7 | (uint8_t)command));

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

    #define SET_METHOD_BIND_REAL(cmd, method) SET_METHOD_BIND_TYPE(cmd, method, real_t)

    #define GET_BIND_VALUE(cmd, value)\
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
        SET_METHOD_BIND_TYPE(Command::CLAMP_POS, setTargetPositionClamp, (std::tuple<real_t, real_t>))
        SET_METHOD_BIND_REAL(Command::CLAMP_SPD, setSpeedClamp)
        SET_METHOD_BIND_REAL(Command::CLAMP_ACC, setAccelClamp)

        GET_BIND_VALUE(Command::GET_POS, est_pos)
        GET_BIND_VALUE(Command::GET_SPD, est_speed)
        GET_BIND_VALUE(Command::GET_ACC, 0)

        SET_METHOD_BIND_EXECUTE(Command::CALI, triggerCali)

        SET_METHOD_BIND_EXECUTE(Command::SAVE, saveArchive, false)
        SET_METHOD_BIND_EXECUTE(Command::LOAD, loadArchive, false)
        SET_METHOD_BIND_EXECUTE(Command::RM, removeArchive, false)

        SET_METHOD_BIND_EXECUTE(Command::INACTIVE, enable, false)
        SET_METHOD_BIND_EXECUTE(Command::ACTIVE, enable, true)
        SET_METHOD_BIND_EXECUTE(Command::SET_NODEID, setNodeId, msg.to<uint8_t>())

        default:
            Cli::parse_command(command, msg);
            break;
    }

    #undef SET_METHOD_BIND
    #undef SET_VALUE_BIND
    #undef SET_METHOD_BIND_REAL
    #undef SET_VALUE_BIND_REAL
}


void Stepper::tick(){
    auto begin_micros = micros();
    RunStatus exe_status = RunStatus::NONE;

    switch(run_status){
        case RunStatus::INIT:
            {
                // static bool load_lock = false;
                bool load_ok = autoload(false);
                if(load_ok){
                    if(skip_tone){
                        panel_led.setTranstit(Color(), Color(0,0,1,0), StatLed::Method::Squ);
                        active_task(true);
                    }
                    else beep_task(true);
                }else{
                    cali_task(true);
                }
                // // bool load_ok = false;
                // if(load_ok){
                //     run_status = RunStatus::CALI;
                //     new_status = RunStatus::EXIT;
                //     logger.println("autoload ok");
                // }else{
                // check_task(true);
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
    if(run_status == RunStatus::ACTIVE and logger.pending() == 0) RUN_DEBUG(target, est_speed, est_pos, run_current, run_leadangle);
    // delay(1);
    // , est_speed, t, odo.getElecRad(), openloop_elecrad);
    // logger << est_pos << est_speed << run_current << elecrad_zerofix << endl;
    // RUN_DEBUG(est_pos, est_speed, run_current, elecrad_zerofix);
    // RUN_DEBUG(est_pos, est_speed, run_current, run_elecrad);

    // bool led_status = (millis() / 200) % 2;
    // bled = led_status;
}