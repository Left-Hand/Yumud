#include "robots/foc/focmotor.hpp"
#include "core/system.hpp"

using namespace ymd;
using namespace ymd::foc;
using namespace ymd::drivers;

#define MY_OS_PRINTS(...)

void FOCMotor::AsciiProtocol::parseArgs(const StringViews args){

    auto targ_pos_setter = [&](const real_t arg, const real_t offset){
        if(args.size() > 1){
            auto val = arg + offset;
            motor.setTargetPosition(val);
            MY_OS_PRINTS("target position:", val, " n");
        }else{
            MY_OS_PRINTS("position:", motor.getPosition(), " n");
        }
    };

    switch(args[0].hash()){
        case "save"_ha:
        case "sv"_ha:

            MY_OS_PRINTS("======");
            MY_OS_PRINTS("saving archive...");

            motor.saveArchive();

            MY_OS_PRINTS("done");
            MY_OS_PRINTS("======");
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
            if(args.size() == 2) motor.setAccLimit(real_t(args[1]));
            break;

        case "remove"_ha:
        case "rm"_ha:

            MY_OS_PRINTS("======");
            MY_OS_PRINTS("removing archive...");

            motor.removeArchive();

            MY_OS_PRINTS("done");
            MY_OS_PRINTS("======");
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
                MY_OS_PRINTS("target speed:", spd, " n/s");
            }else{
                MY_OS_PRINTS("speed:", motor.getSpeed(), " n/s");
            }
            break;

        case "position"_ha:
        case "pos"_ha:
        case "p"_ha:
            targ_pos_setter(real_t(args[1]), real_t(0));
            break;
        case "f"_ha:
            motor.setTargetPositionDelta(real_t(args[1]));
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
                MY_OS_PRINTS("target teach:", val);
            }
            break;

        case "curr"_ha:
        case "c"_ha:
            if(args.size() > 1){
                real_t val = real_t(args[1]);
                motor.setTargetCurrent(val);
                MY_OS_PRINTS("target current:", val, " n");
            }else{
                MY_OS_PRINTS("current:", motor.getCurrent(), " A");
            }
            break;

        case "vect"_ha:
        case "v"_ha:
            if(args.size() > 1){
                auto v = real_t(args[1]);
                motor.setTargetVector(v);
                MY_OS_PRINTS("target vector:", v, " n");
            }
            break;

        case "crc"_ha:
            MY_OS_PRINTS(std::hex, std::showbase, sys::Chip::getChipIdCrc());
            break;

        case "eleczero"_ha:
        case "ez"_ha:{
            if(args.size() <= 1) break;
            motor.elecrad_zerofix = real_t(args[1]);
            break;
        }

        case "error"_ha:
        case "err"_ha:
            if(motor.getErrMsg()) {MY_OS_PRINTS(motor.getErrMsg());}
            else {MY_OS_PRINTS("no error");}
            break;

        case "warn"_ha:
        case "wa"_ha:
            if(motor.getWarnMsg()) {MY_OS_PRINTS(motor.getWarnMsg());}
            else {MY_OS_PRINTS("no warn");}
            break;

        case "enable"_ha:
        case "en"_ha:
        case "e"_ha:
            motor.enable();
            MY_OS_PRINTS("enabled");
            break;
        
        case "exe"_ha:
            MY_OS_PRINTS("exe", motor.exe(), "us");
            break;

        case "disable"_ha:
        case "dis"_ha:
        case "de"_ha:
        case "d"_ha:
            motor.enable(DISEN);
            MY_OS_PRINTS("disabled");
            break;

        case "cali"_ha:
            motor.triggerCali();
            MY_OS_PRINTS("cali started");
            break;

        case "locate"_ha:
        case "loc"_ha:
        {
            real_t loc = (args.size() > 1)? real_t(args[1]) : 0.0_r;
            motor.locateRelatively(loc);
            MY_OS_PRINTS("located to", loc);
        }
            break;

        // case "beep"_ha:
        //     motor.beep_task(true);
        //     break;

        case "id"_ha:
            MY_OS_PRINTS("node id is: ", uint8_t(motor.getNodeId()));
            break;

        // case "rd"_ha:
        //     motor.run_debug_enabled = (args.size() > 1)? int(args[1]) : true;
        //     MY_OS_PRINTS("run debug enabled:", run_debug_enabled);
        //     break;

        // case "cl"_ha:{
        //     auto cl = (args.size() > 1)? real_t(args[1]) : 0;
        //     MY_OS_PRINTS("current clamp:", cl);
        // }
        //     break;

        // case "status"_ha:
        // case "stat"_ha:
        //     MY_OS_PRINTS("current status:", int(motor.run_status));
        //     break;

        // case "cd"_ha:
        //     MY_OS_PRINTS("dir changed");
        //     motor.elecrad_zerofix = real_t(PI);
        //     break;

        case "hlt"_ha:
            MY_OS_PRINTS("halt");
            PANIC();
            break;

        case "map"_ha:
            // for(const auto & item : motor.odo.map()){
            //     MY_OS_PRINTS(item);
            //     clock::delay(1ms);
            // }
            
            break;

        case "cm"_ha:
            // motor.odo.map().fill(0);
            break;

        // case "version"_ha:
        // case "ver"_ha:
        //     break;
        // case "info"_ha:
        //     MY_OS_PRINTS(motor.archive_.board_info);
        //     break;

        default:
            AsciiProtocolConcept::parseArgs(args);
            break;
    }
}

