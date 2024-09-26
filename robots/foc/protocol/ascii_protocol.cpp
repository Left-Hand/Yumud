#include "ascii_protocol.hpp"
#include "robots/foc/focmotor.hpp"


void AsciiProtocol::parseTokens(const StringView _command, const Strings & args){
    auto command = _command;
    switch(hash_impl(command.data(), command.length())){
        case "save"_ha:
        case "sv"_ha:
            motor.saveArchive(args.size() ? bool(int(args[0])) : true);
            break;

        case "load"_ha:
        case "ld"_ha:
            motor.loadArchive(args.size() ? bool(int(args[0])) : true);
            break;

        case "lp"_ha:
            if(args.size() == 2)motor.setPositionLimit(Range{args[0], args[1]});
            // else CLI_DEBUG(motor.get)
            break;
        
        case "lc"_ha:
            if(args.size() == 1) motor.setCurrentLimit(real_t(args[0]));
            break;

        case "ls"_ha:
            if(args.size() == 1) motor.setSpeedLimit(real_t(args[0]));
            break;

        case "la"_ha:
            if(args.size() == 1) motor.setAccelLimit(real_t(args[0]));
            break;

        case "remove"_ha:
        case "rm"_ha:
            motor.removeArchive(args.size() ? bool(int(args[0])) : true);
            break;

        case "speed"_ha:
        case "spd"_ha:
        case "s"_ha:
            if(args.size()){
                real_t spd = real_t(args[0]);
                motor.setTargetSpeed(spd);
                CLI_DEBUG("target speed:", spd, " n/s");
            }else{
                CLI_DEBUG("speed:", motor.getSpeed(), " n/s");
            }
            break;

        case "position"_ha:
        case "pos"_ha:
        case "p"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                motor.setTargetPosition(val);
                CLI_DEBUG("target position:", val, " n");
            }else{
                CLI_DEBUG("position:", motor.getPosition(), " n");
            }
            break;


        case "teach"_ha:
        case "tch"_ha:
        case "th"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                motor.setTargetTeach(val);
                CLI_DEBUG("target teach:", val);
            }
            break;

        case "tpz"_ha:
        case "t"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                motor.setTargetTrapezoid(val);
                CLI_DEBUG("target trapezoid:", val, " n");
            }else{
                CLI_DEBUG("position:", motor.getPosition(), " n");
            }
            break;

        case "curr"_ha:
        case "c"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                motor.setTargetCurrent(val);
                CLI_DEBUG("target current:", val, " n");
            }else{
                CLI_DEBUG("current:", motor.getCurrent(), " A");
            }
            break;

        case "vect"_ha:
        case "v"_ha:
            if(args.size()){
                auto v = real_t(args[0]);
                motor.setTargetVector(v);
                CLI_DEBUG("target vector:", v, " n");
            }
            break;

        case "crc"_ha:
            CLI_DEBUG(std::hex, Sys::Chip::getChipIdCrc());
            break;

        // case "eleczero"_ha:
        // case "ez"_ha:{
        //     if(args.size() == 0) break;
        //     motor.elecrad_zerofix = real_t(args[0]);
        // }
        //     break;

        case "error"_ha:
        case "err"_ha:
            if(motor.getErrMsg()) {CLI_DEBUG(motor.getErrMsg())}
            else {CLI_DEBUG("no error")}
            break;

        case "warn"_ha:
        case "wa"_ha:
            if(motor.getWarnMsg()) {CLI_DEBUG(motor.getWarnMsg())}
            else {CLI_DEBUG("no warn")}
            break;

        case "enable"_ha:
        case "en"_ha:
        case "e"_ha:
            motor.enable();
            CLI_DEBUG("enabled");
            break;
        
        case "exe"_ha:
            CLI_DEBUG("exe", motor.exe(), "us");
            break;

        case "disable"_ha:
        case "dis"_ha:
        case "de"_ha:
        case "d"_ha:
            motor.enable(false);
            CLI_DEBUG("disabled");
            break;

        case "cali"_ha:
            motor.triggerCali();
            CLI_DEBUG("cali started");
            break;

        case "locate"_ha:
        case "loc"_ha:
        {
            real_t loc = args.size() ? real_t(args[0]) : 0;
            motor.locateRelatively(loc);
            CLI_DEBUG("located to", loc);
        }
            break;

        // case "beep"_ha:
        //     motor.beep_task(true);
        //     break;

        case "id"_ha:
            CLI_DEBUG("node id is: ", uint8_t(motor.getNodeId()));
            break;

        // case "rd"_ha:
        //     motor.run_debug_enabled = args.size() ? int(args[0]) : true;
        //     CLI_DEBUG("run debug enabled:", run_debug_enabled);
        //     break;

        // case "cl"_ha:{
        //     auto cl = args.size() ? real_t(args[0]) : 0;
        //     CLI_DEBUG("current clamp:", cl);
        // }
        //     break;

        // case "status"_ha:
        // case "stat"_ha:
        //     CLI_DEBUG("current status:", int(motor.run_status));
        //     break;

        // case "cd"_ha:
        //     CLI_DEBUG("dir changed");
        //     motor.elecrad_zerofix = real_t(PI);
        //     break;

        case "hlt"_ha:
            CLI_DEBUG("halt");
            CREATE_FAULT;
            break;

        case "map"_ha:
            for(const auto & item : motor.odo.map()){
                CLI_DEBUG(item);
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
        //     CLI_DEBUG(motor.archive_.board_info);
        //     break;
        case "reset"_ha:
        case "rst"_ha:
        case "r"_ha:
            CLI_DEBUG("rsting");
            NVIC_SystemReset();
            break;
        case "alive"_ha:
        case "a"_ha:
            CLI_DEBUG("chip is alive");
            break;
        default:
            CLI_DEBUG("no command available:", command);
            break;
    }
}

void AsciiProtocol::parseLine(const StringView _line){
    auto tokens = _line.split(' ');
    if(tokens.size()){
        auto command = tokens[0];
        tokens.erase(tokens.begin());
        parseTokens(command, tokens);
    }
}

void AsciiProtocol::readString(){
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
