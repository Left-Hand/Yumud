#include "robots/foc/focmotor.hpp"

using namespace ymd;
using namespace ymd::foc;
using namespace ymd::drivers;

void FOCMotor::AsciiProtocol::parseArgs(const Strings & args){

    auto targ_pos_setter = [&](const real_t arg, const real_t offset){
        if(args.size() > 1){
            auto val = arg + offset;
            motor.setTargetPosition(val);
            os.prints("target position:", val, " n");
        }else{
            os.prints("position:", motor.getPosition(), " n");
        }
    };

    switch(args[0].hash()){
        case "save"_ha:
        case "sv"_ha:

            os.prints("======");
            os.prints("saving archive...");

            motor.saveArchive();

            os.prints("done");
            os.prints("======");
            break;

        case "pos.kp"_ha:
            break;
        
        case "load"_ha:
        case "ld"_ha:
            motor.loadArchive();
            break;

        case "lp"_ha:
            if(args.size() == 3)motor.setPositionLimit(Range{real_t(args[1]), real_t(args[2])});
            if(args.size() == 2)motor.setPositionLimit(Range{-real_t(args[1]), real_t(args[1])});
            break;
        
        case "lc"_ha:
            if(args.size() == 2) motor.setCurrentLimit(real_t(args[1]));
            break;

        case "ls"_ha:
            if(args.size() == 2) motor.setSpeedLimit(real_t(args[1]));
            break;

        case "la"_ha:
            if(args.size() == 2) motor.setAccelLimit(real_t(args[1]));
            break;

        case "remove"_ha:
        case "rm"_ha:

            os.prints("======");
            os.prints("removing archive...");

            motor.removeArchive();

            os.prints("done");
            os.prints("======");
            break;


        case "pkp"_ha:
            motor.archive().pos_config.kp = real_t(args[1]);
            break;
        case "pkd"_ha:
            motor.archive().pos_config.kd = real_t(args[1]);
            break;

        case "skp"_ha:
            motor.archive().spd_config.kp = real_t(args[1]);
            break;
        case "skd"_ha:
            motor.archive().spd_config.kd = real_t(args[1]);
            break;

        case "rf"_ha:
            motor.setRadfix(real_t(args[1]));
            break;
        
        case "speed"_ha:
        case "spd"_ha:
        case "s"_ha:
            if(args.size() > 1){
                real_t spd = real_t(args[1]);
                motor.setTargetSpeed(spd);
                os.prints("target speed:", spd, " n/s");
            }else{
                os.prints("speed:", motor.getSpeed(), " n/s");
            }
            break;

        case "position"_ha:
        case "pos"_ha:
        case "p"_ha:
            targ_pos_setter(real_t(args[1]), real_t(0));
            break;
        case "f"_ha:
            targ_pos_setter(real_t(args[1]), motor.getPosition());
            break;
        case "b"_ha:
            targ_pos_setter(-real_t(args[1]), motor.getPosition());
            break;
        case "teach"_ha:
        case "tch"_ha:
        case "th"_ha:
            if(args.size() > 1){
                real_t val = real_t(args[1]);
                motor.setTargetTeach(val);
                os.prints("target teach:", val);
            }
            break;

        case "curr"_ha:
        case "c"_ha:
            if(args.size() > 1){
                real_t val = real_t(args[1]);
                motor.setTargetCurrent(val);
                os.prints("target current:", val, " n");
            }else{
                os.prints("current:", motor.getCurrent(), " A");
            }
            break;

        case "vect"_ha:
        case "v"_ha:
            if(args.size() > 1){
                auto v = real_t(args[1]);
                motor.setTargetVector(v);
                os.prints("target vector:", v, " n");
            }
            break;

        case "crc"_ha:
            os.prints(std::hex, std::showbase, Sys::Chip::getChipIdCrc());
            break;

        case "eleczero"_ha:
        case "ez"_ha:{
            if(args.size() <= 1) break;
            motor.elecrad_zerofix = real_t(args[1]);
            break;
        }

        case "error"_ha:
        case "err"_ha:
            if(motor.getErrMsg()) {os.prints(motor.getErrMsg());}
            else {os.prints("no error");}
            break;

        case "warn"_ha:
        case "wa"_ha:
            if(motor.getWarnMsg()) {os.prints(motor.getWarnMsg());}
            else {os.prints("no warn");}
            break;

        case "enable"_ha:
        case "en"_ha:
        case "e"_ha:
            motor.enable();
            os.prints("enabled");
            break;
        
        case "exe"_ha:
            os.prints("exe", motor.exe(), "us");
            break;

        case "disable"_ha:
        case "dis"_ha:
        case "de"_ha:
        case "d"_ha:
            motor.enable(false);
            os.prints("disabled");
            break;

        case "cali"_ha:
            motor.triggerCali();
            os.prints("cali started");
            break;

        case "locate"_ha:
        case "loc"_ha:
        {
            real_t loc = (args.size() > 1)? real_t(args[1]) : 0;
            motor.locateRelatively(loc);
            os.prints("located to", loc);
        }
            break;

        // case "beep"_ha:
        //     motor.beep_task(true);
        //     break;

        case "id"_ha:
            os.prints("node id is: ", uint8_t(motor.getNodeId()));
            break;

        // case "rd"_ha:
        //     motor.run_debug_enabled = (args.size() > 1)? int(args[1]) : true;
        //     os.prints("run debug enabled:", run_debug_enabled);
        //     break;

        // case "cl"_ha:{
        //     auto cl = (args.size() > 1)? real_t(args[1]) : 0;
        //     os.prints("current clamp:", cl);
        // }
        //     break;

        // case "status"_ha:
        // case "stat"_ha:
        //     os.prints("current status:", int(motor.run_status));
        //     break;

        // case "cd"_ha:
        //     os.prints("dir changed");
        //     motor.elecrad_zerofix = real_t(PI);
        //     break;

        case "hlt"_ha:
            os.prints("halt");
            PANIC();
            break;

        case "map"_ha:
            // for(const auto & item : motor.odo.map()){
            //     os.prints(item);
            //     delay(1);
            // }
            
            break;

        case "cm"_ha:
            // motor.odo.map().fill(0);
            break;

        // case "version"_ha:
        // case "ver"_ha:
        //     break;
        // case "info"_ha:
        //     os.prints(motor.archive_.board_info);
        //     break;

        default:
            AsciiProtocolConcept::parseArgs(args);
            break;
    }
}

