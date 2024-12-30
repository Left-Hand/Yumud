#pragma once

#include "sys/stream/ostream.hpp"
#include "debug_inst.hpp"

namespace ymd{

class Debugger:public OutputStream{
protected:
    DebuggerType * _os;
    Debugger(DebuggerType & os):
        _os(&os){}
public:
    static Debugger & singleton();

    void init();
    void init(const uint baud);

    void change(DebuggerType & os){
        _os = &os;
    }
    void write(const char data) override final{
        _os->write(data);
    }
    void write(const char * pdata, const size_t len) override final{
        _os->write(pdata, len);
	}

    size_t pending() const {
        return _os->pending();
    }    
};

}