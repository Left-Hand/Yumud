#include "ascii_protocol.hpp"
#include "robots/foc/focmotor.hpp"


void AsciiProtocol::parseTokens(const String & _command, const std::vector<String> & args){
    auto command = _command;
    switch(hash_impl(command.c_str(), command.length())){
        case "save"_ha:
        case "sv"_ha:
            motor.saveArchive(args.size() ? bool(int(args[0])) : true);
            break;

        case "load"_ha:
        case "ld"_ha:
            motor.loadArchive(args.size() ? bool(int(args[0])) : true);
            break;

        case "nz"_ha:
            {
                real_t val = args.size() ? int(args[0]) : 0;
                motor.setNozzle(val);
                CLI_DEBUG("set nozzle to:",val);
            }
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
                CLI_DEBUG("targ speed\t", spd, " n/s");
            }else{
                CLI_DEBUG("curr speed\t", motor.getSpeed(), " n/s");
            }
            break;

        case "position"_ha:
        case "pos"_ha:
        case "p"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                motor.setTargetPosition(val);
                CLI_DEBUG("targ position\t", val, " n");
            }else{
                CLI_DEBUG("now position", motor.getPosition(), " n");
            }
            break;


        case "teach"_ha:
        case "tch"_ha:
        case "th"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                motor.setTargetTeach(val);
                CLI_DEBUG("targ teach\t", val);
            }
            break;

        case "tpz"_ha:
        case "t"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                motor.setTargetTrapezoid(val);
                CLI_DEBUG("targ position\t", val, " n");
            }else{
                CLI_DEBUG("now position\t", motor.getPosition(), " n");
            }
            break;

        case "curr"_ha:
        case "c"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                motor.setTargetCurrent(val);
                CLI_DEBUG("targ current\t", val, " n");
            }else{
                CLI_DEBUG("now current\t", motor.getCurrent(), " A");
            }
            break;

        case "vect"_ha:
        case "v"_ha:
            if(args.size()){
                auto v = real_t(args[0]);
                motor.setTargetVector(v);
                CLI_DEBUG("targ vector\t", v, " n");
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

        // case "error"_ha:
        // case "err"_ha:
        //     if(motor.error_message) {CLI_DEBUG(error_message)}
        //     else {CLI_DEBUG("no error")}
        //     break;

        // case "warn"_ha:
        // case "wa"_ha:
        //     if(motor.warn_message) {CLI_DEBUG(warn_message)}
        //     else {CLI_DEBUG("no warn")}
        //     break;

        // case "enable"_ha:
        // case "en"_ha:
        // case "e"_ha:
        //     motor.rework();
        //     CLI_DEBUG("enabled");
        //     break;
        
        // case "exe"_ha:
        //     CLI_DEBUG("exe", motor.exe_micros, "us");
        //     break;

        // case "disable"_ha:
        // case "dis"_ha:
        // case "de"_ha:
        // case "d"_ha:
        //     motor.shutdown();
        //     CLI_DEBUG("disabled");
        //     break;

        // case "cali"_ha:
        //     motor.cali_task(true);
        //     CLI_DEBUG("cali started");
        //     break;

        // case "locate"_ha:
        // case "loc"_ha:
        // {
        //     real_t loc = args.size() ? real_t(args[0]) : 0;
        //     locateRelatively(loc);
        //     CLI_DEBUG("located to", loc);
        // }
        //     break;

        // case "beep"_ha:
        //     motor.beep_task(true);
        //     break;

        // case "id"_ha:
        //     CLI_DEBUG("node id is: ", uint8_t(motor.node_id));
        //     break;

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

        // case "shutdown"_ha:
        // case "shut"_ha:
        //     motor.shutdown();
        //     CLI_DEBUG("shutdown ok");
        //     break;

        // case "cd"_ha:
        //     CLI_DEBUG("dir changed");
        //     motor.elecrad_zerofix = real_t(PI);
        //     break;

        // case "hlt"_ha:
        //     CLI_DEBUG("halt");
        //     CREATE_FAULT;
        //     break;

        // case "map"_ha:
        //     for(const auto & item : motor.odo.map()){
        //         CLI_DEBUG(item);
        //         delay(1);
        //     }
        //     break;

        // case "version"_ha:
        // case "ver"_ha:
        //     break;
        // case "info"_ha:
        //     CLI_DEBUG(motor.archive_.board_info);
        //     break;
    }
}

void AsciiProtocol::parseLine(const String & _line){
    if(_line.length() < 1) return;

    auto tokens = split_string(_line, ' ');
    if(tokens.size()){
        auto command = tokens[0];
        tokens.erase(tokens.begin());
        parseTokens(command, tokens);
    }
}


std::vector<String> AsciiProtocol::split_string(const String& input, char delimiter) {
    std::vector<String> result;

    int startPos = 0;
    int endPos = input.indexOf(delimiter, startPos);

    while (endPos != -1) {
        if(not(endPos - startPos <= 1 and input[startPos] == delimiter)){
            String token = input.substring(startPos, endPos);
            result.push_back(token.c_str());
        }
        startPos = endPos + 1;
        endPos = input.indexOf(delimiter, startPos);
    }

    if (startPos < (int)input.length()) {
        String lastToken = input.substring(startPos);
        result.push_back(lastToken.c_str());
    }

    return result;
}


void AsciiProtocol::readString(){
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
