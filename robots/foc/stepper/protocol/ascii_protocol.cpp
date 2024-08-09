#include "../stepper.hpp"
#include "ascii_protocol.hpp"




void FOCStepper::parseTokens(const String & _command, const std::vector<String> & args){
    auto command = _command;
    switch(hash_impl(command.c_str(), command.length())){
        case "save"_ha:
        case "sv"_ha:
            saveArchive(args.size() ? bool(int(args[0])) : true);
            break;

        case "load"_ha:
        case "ld"_ha:
            loadArchive(args.size() ? bool(int(args[0])) : true);
            break;

        case "nz"_ha:
            {
                real_t val = args.size() ? int(args[0]) : 0;
                setNozzle(val);
                CLI_DEBUG("set nozzle to:",val);
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
                CLI_DEBUG("targ speed\t", spd, " n/s");
            }else{
                CLI_DEBUG("curr speed\t", getSpeed(), " n/s");
            }
            break;

        case "position"_ha:
        case "pos"_ha:
        case "p"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                setTargetPosition(val);
                CLI_DEBUG("targ position\t", val, " n");
            }else{
                CLI_DEBUG("now position", getPosition(), " n");
            }
            break;

        case "tpz"_ha:
        case "t"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                setTargetTrapezoid(val);
                CLI_DEBUG("targ position\t", val, " n");
            }else{
                CLI_DEBUG("now position\t", getPosition(), " n");
            }
            break;

        case "stable"_ha:
            CLI_DEBUG(odo.encoder.stable());
            break;

        case "curr"_ha:
        case "c"_ha:
            if(args.size()){
                real_t val = real_t(args[0]);
                setTargetCurrent(val);
                CLI_DEBUG("targ current\t", val, " n");
            }else{
                CLI_DEBUG("now current\t", getCurrent(), " n");
            }
            break;

        case "vect"_ha:
        case "v"_ha:
            if(args.size()){
                auto v = real_t(args[0]);
                setTargetVector(v);
                CLI_DEBUG("targ vector\t", v, " n");
            }
            break;

        case "crc"_ha:
            CLI_DEBUG(std::hex, Sys::Chip::getChipIdCrc());
            break;

        case "eleczero"_ha:
        case "ez"_ha:{
            if(args.size() == 0) break;
            elecrad_zerofix = real_t(args[0]);
        }
            break;

        case "error"_ha:
        case "err"_ha:
            if(error_message) {CLI_DEBUG(error_message)}
            else {CLI_DEBUG("no error")}
            break;

        case "warn"_ha:
        case "wa"_ha:
            if(warn_message) {CLI_DEBUG(warn_message)}
            else {CLI_DEBUG("no warn")}
            break;

        case "enable"_ha:
        case "en"_ha:
        case "e"_ha:
            rework();
            CLI_DEBUG("enabled");
            break;
        
        case "exe"_ha:
            CLI_DEBUG("exe", exe_micros, "us");
            break;

        case "disable"_ha:
        case "dis"_ha:
        case "de"_ha:
        case "d"_ha:
            shutdown();
            CLI_DEBUG("disabled");
            break;

        case "cali"_ha:
            cali_task(true);
            CLI_DEBUG("cali started");
            break;

        case "locate"_ha:
        case "loc"_ha:
        {
            real_t loc = args.size() ? real_t(args[0]) : 0;
            locateRelatively(loc);
            CLI_DEBUG("located to", loc);
        }
            break;

        case "beep"_ha:
            beep_task(true);
            break;

        case "id"_ha:
            CLI_DEBUG("node id is: ", uint8_t(node_id));
            break;

        case "rd"_ha:
            run_debug_enabled = args.size() ? int(args[0]) : true;
            CLI_DEBUG("run debug enabled:", run_debug_enabled);
            break;

        case "cl"_ha:{
            auto cl = args.size() ? real_t(args[0]) : 0;
            CLI_DEBUG("current clamp:", cl);
        }
            break;

        case "status"_ha:
        case "stat"_ha:
            CLI_DEBUG("current status:", int(run_status));
            break;

        case "shutdown"_ha:
        case "shut"_ha:
            shutdown();
            CLI_DEBUG("shutdown ok");
            break;

        case "cd"_ha:
            CLI_DEBUG("dir changed");
            elecrad_zerofix = real_t(PI);
            break;

        case "hlt"_ha:
            CLI_DEBUG("halt");
            CREATE_FAULT;
            break;

        case "map"_ha:
            for(const auto & item : odo.map()){
                CLI_DEBUG(item);
                delay(1);
            }
            break;

        case "version"_ha:
        case "ver"_ha:
            break;
        case "info"_ha:
            CLI_DEBUG(archive_.board_info);
            break;
        default:
            CliSTA::parseTokens(command, args);
            break;
    }
}