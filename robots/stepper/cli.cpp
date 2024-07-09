#include "cli.hpp"

using namespace StepperUtils;

void CliSTA::read_can(){
    if(can.available()){
        const CanMsg & msg = can.read();
        uint8_t id = msg.id() >> 7;
        Command cmd = (Command)(msg.id() & 0x7F);
        if(id == 0 || id == node_id){
            parse_command(cmd, msg);
        }
    }
}