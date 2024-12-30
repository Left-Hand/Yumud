#include "debugger.hpp"
#include "debug_inst.hpp"

namespace ymd{

void Debugger::init(const uint baud){
    _os->init(baud);
}

void Debugger::init(){
    _os->init(DEBUG_UART_BAUD);
}

Debugger & Debugger::singleton(){
    static Debugger dbg(DEBUGGER_INST);
    return dbg;
}

Debugger & DEBUGGER = Debugger::singleton();
OutputStream & LOGGER = LOGGER_INST;

}