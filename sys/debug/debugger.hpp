#pragma once

#include "sys/stream/ostream.hpp"
#include "debug_inst.hpp"

namespace ymd{

class __Debugger:public OutputStream{
protected:
    #ifndef YMD_DEFAULT_DEBUGGER_ROUTE_TYPE
    #define YMD_DEFAULT_DEBUGGER_ROUTE_TYPE UartHw
    #endif

    OutputStream * route_;
    __Debugger(OutputStream & route):
        route_(&route){}
public:
    static __Debugger & singleton();

    template<typename T, typename ... Args>
    void init(Args && ... args){
        reinterpret_cast<T *>(route_)->init(args...);
    }
    
    template<typename T = YMD_DEFAULT_DEBUGGER_ROUTE_TYPE>
    void init(){
        if constexpr(std::is_base_of_v<Uart, T>){
            reinterpret_cast<T *>(route_)->init(DEBUG_UART_BAUD);
        }
    }

    void retarget(OutputStream & route){
        route_ = &route;
    }

    template<typename T>
    T & route() {return reinterpret_cast<T &>(*route_);}

    void write(const char data) override final{
        route_->write(data);
    }
    void write(const char * pdata, const size_t len) override final{
        route_->write(pdata, len);
	}

    size_t pending() const {
        return route_->pending();
    }    
};

}