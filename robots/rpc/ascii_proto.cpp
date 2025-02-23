#include "ascii_proto.hpp"
#include "sys/core/system.hpp"


using namespace ymd;

void AsciiProtocolConcept::parseArgs(const StringViews args){
    switch(args[0].hash()){
        case "reset"_ha:
        case "rst"_ha:
        case "r"_ha:
            os.prints("rsting");
            sys::reset();
            break;
        case "alive"_ha:
        case "a"_ha:
            os.prints("chip is alive");
            break;
        default:
            os.prints("no command available:", args[0]);
            break;
    }
}
