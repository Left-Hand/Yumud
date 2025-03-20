#pragma once

#include "core/stream/ostream.hpp"
#include "core/stream/CharOpTraits.hpp"
#include "debug_inst.hpp"

namespace ymd{

class __Debugger final:public OutputStream{
private:

    using Traits = WriteCharTraits;
    using Route = pro::proxy<Traits>;
    Route p_route_;
    __Debugger() = default;

    __fast_inline void check() const {
        if(!p_route_) HALT;
    } 
public:
    static __Debugger & singleton();

    template<typename T = DebuggerType, typename ... Args>
    constexpr __Debugger & init(T & route, Args && ... args){
        route.init(args...);

        p_route_ = pro::make_proxy<Traits>(&route);
        return *this;
    }

    void retarget(Route p_route){
        p_route_ = p_route;
    }

    Route & route() {
        check();
        return p_route_;
    }

    void write(const char data) override final{
        check();
        p_route_->write1(data);
    }
    void write(const char * pdata, const size_t len) override final{
        check();
        p_route_->writeN(pdata, len);
	}

    size_t pending() const {
        check();
        return p_route_->pending();
    }


};

}