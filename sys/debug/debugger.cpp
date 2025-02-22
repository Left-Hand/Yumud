#include "debugger.hpp"
#include "debug_inst.hpp"

namespace ymd{

__Debugger & __Debugger::singleton(){
    static __Debugger dbg(DEBUGGER_INST);
    return dbg;
}

__Debugger & DEBUGGER = __Debugger::singleton();
OutputStream & LOGGER = LOGGER_INST;

}