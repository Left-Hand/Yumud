#include "debugger.hpp"

namespace ymd{

__Debugger & __Debugger::singleton(){
    static __Debugger dbg;
    return dbg;
}

__Debugger & DEBUGGER = __Debugger::singleton();
// OutputStream & LOGGER = LOGGER_INST;
}