#include "ascii_proto.hpp"

void AsciiProtocolConcept::parseArgs(const Strings & args){
    switch(args[0].hash()){
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
            CLI_DEBUG("no command available:", args[0]);
            break;
    }
}
