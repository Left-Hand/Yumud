#include "robots/foc/focmotor.hpp"


void FOCMotor::AsciiProtocol::parseArgs(const Strings & args){
    switch(args[0].hash()){
        case "save"_ha:
        case "sv"_ha:

            os.println("======");
            os.println("saving archive...");

            motor.saveArchive();

            os.println("done");
            os.println("======");
            break;

        case "load"_ha:
        case "ld"_ha:
            motor.loadArchive();
            break;

        case "lp"_ha:
            if(args.size() == 2)motor.setPositionLimit(Range{args[1], args[2]});
            break;
        
        case "lc"_ha:
            if(args.size() == 1) motor.setCurrentLimit(real_t(args[1]));
            break;

        case "ls"_ha:
            if(args.size() == 1) motor.setSpeedLimit(real_t(args[1]));
            break;

        case "la"_ha:
            if(args.size() == 1) motor.setAccelLimit(real_t(args[1]));
            break;

        case "remove"_ha:
        case "rm"_ha:

            os.println("======");
            os.println("removing archive...");

            motor.removeArchive();

            os.println("done");
            os.println("======");
            break;

        case "speed"_ha:
        case "spd"_ha:
        case "s"_ha:
            if(args.size() > 1){
                real_t spd = real_t(args[1]);
                motor.setTargetSpeed(spd);
                os.println("target speed:", spd, " n/s");
            }else{
                os.println("speed:", motor.getSpeed(), " n/s");
            }
            break;

        case "position"_ha:
        case "pos"_ha:
        case "p"_ha:
            if(args.size() > 1){
                real_t val = real_t(args[1]);
                motor.setTargetPosition(val);
                os.println("target position:", val, " n");
            }else{
                os.println("position:", motor.getPosition(), " n");
            }
            break;


        case "teach"_ha:
        case "tch"_ha:
        case "th"_ha:
            if(args.size() > 1){
                real_t val = real_t(args[1]);
                motor.setTargetTeach(val);
                os.println("target teach:", val);
            }
            break;

        case "curr"_ha:
        case "c"_ha:
            if(args.size() > 1){
                real_t val = real_t(args[1]);
                motor.setTargetCurrent(val);
                os.println("target current:", val, " n");
            }else{
                os.println("current:", motor.getCurrent(), " A");
            }
            break;

        case "vect"_ha:
        case "v"_ha:
            if(args.size() > 1){
                auto v = real_t(args[1]);
                motor.setTargetVector(v);
                os.println("target vector:", v, " n");
            }
            break;

        case "crc"_ha:
            os.println(std::hex, Sys::Chip::getChipIdCrc());
            break;

        // case "eleczero"_ha:
        // case "ez"_ha:{
        //     if(args.size() == 0) break;
        //     motor.elecrad_zerofix = real_t(args[1]);
        // }
        //     break;

        case "error"_ha:
        case "err"_ha:
            if(motor.getErrMsg()) {os.println(motor.getErrMsg());}
            else {os.println("no error");}
            break;

        case "warn"_ha:
        case "wa"_ha:
            if(motor.getWarnMsg()) {os.println(motor.getWarnMsg());}
            else {os.println("no warn");}
            break;

        case "enable"_ha:
        case "en"_ha:
        case "e"_ha:
            motor.enable();
            os.println("enabled");
            break;
        
        case "exe"_ha:
            os.println("exe", motor.exe(), "us");
            break;

        case "disable"_ha:
        case "dis"_ha:
        case "de"_ha:
        case "d"_ha:
            motor.enable(false);
            os.println("disabled");
            break;

        case "cali"_ha:
            motor.triggerCali();
            os.println("cali started");
            break;

        case "locate"_ha:
        case "loc"_ha:
        {
            real_t loc = (args.size() > 1)? real_t(args[1]) : 0;
            motor.locateRelatively(loc);
            os.println("located to", loc);
        }
            break;

        // case "beep"_ha:
        //     motor.beep_task(true);
        //     break;

        case "id"_ha:
            os.println("node id is: ", uint8_t(motor.getNodeId()));
            break;

        // case "rd"_ha:
        //     motor.run_debug_enabled = (args.size() > 1)? int(args[1]) : true;
        //     os.println("run debug enabled:", run_debug_enabled);
        //     break;

        // case "cl"_ha:{
        //     auto cl = (args.size() > 1)? real_t(args[1]) : 0;
        //     os.println("current clamp:", cl);
        // }
        //     break;

        // case "status"_ha:
        // case "stat"_ha:
        //     os.println("current status:", int(motor.run_status));
        //     break;

        // case "cd"_ha:
        //     os.println("dir changed");
        //     motor.elecrad_zerofix = real_t(PI);
        //     break;

        case "hlt"_ha:
            os.println("halt");
            CREATE_FAULT;
            break;

        case "map"_ha:
            for(const auto & item : motor.odo.map()){
                os.println(item);
                delay(1);
            }
            break;

        case "cm"_ha:
            // motor.odo.map().fill(0);
            break;

        // case "version"_ha:
        // case "ver"_ha:
        //     break;
        // case "info"_ha:
        //     os.println(motor.archive_.board_info);
        //     break;

        default:
            AsciiProtocolConcept::parseArgs(args);
            break;
    }
}

