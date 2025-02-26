#include "ascii_proto.hpp"
#include "sys/core/system.hpp"


using namespace ymd;

#define MY_OS_PRINTS(...)

void AsciiProtocolConcept::parseArgs(const StringViews args){
    switch(args[0].hash()){
        case "reset"_ha:
        case "rst"_ha:
        case "r"_ha:
            MY_OS_PRINTS("rsting");
            sys::reset();
            break;
        case "alive"_ha:
        case "a"_ha:
            MY_OS_PRINTS("chip is alive");
            break;
        default:
            MY_OS_PRINTS("no command available:", args[0]);
            break;
    }
}
