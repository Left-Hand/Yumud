#pragma once

#include "sys/stream/ostream.hpp"
#include "debug_inst.hpp"

namespace ymd{

class __Debugger final:public OutputStream{
private:
    #ifndef YMD_DEFAULT_DEBUGGER_ROUTE_TYPE
    #define YMD_DEFAULT_DEBUGGER_ROUTE_TYPE UartHw
    #endif

    using Route = OutputStream;
    Route * p_route_ = nullptr;
    __Debugger() = default;

    __fast_inline void check() const {
        if(!p_route_) HALT;
    } 
public:
    static __Debugger & singleton();

    template<typename T = YMD_DEFAULT_DEBUGGER_ROUTE_TYPE, typename ... Args>
    constexpr __Debugger & init(T & route, Args && ... args){
        route.init(args...);

        p_route_ = reinterpret_cast<Route *>(&route);
        return *this;
    }
    
    // template<typename T = YMD_DEFAULT_DEBUGGER_ROUTE_TYPE>
    // void init(){
    //     if constexpr(std::is_base_of_v<Uart, T>){
    //         reinterpret_cast<T *>(p_route_)->init(DEBUG_UART_BAUD);
    //     }
    // }

    constexpr void retarget(OutputStream & route){
        p_route_ = &route;
    }

    template<typename T>
    T & route() {
        check();
        return reinterpret_cast<T &>(*p_route_);
    }

    void write(const char data) override final{
        check();
        p_route_->write(data);
    }
    void write(const char * pdata, const size_t len) override final{
        check();
        p_route_->write(pdata, len);
	}

    size_t pending() const {
        check();
        return p_route_->pending();
    }


};

}