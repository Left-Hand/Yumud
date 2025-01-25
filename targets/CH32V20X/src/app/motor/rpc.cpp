#include "rpc.hpp"
#include "tb.h"
#include "sys/debug/debug_inc.h"

using namespace ymd;

class Ball{
public:
    void set_xy(iq_t x, iq_t y){
        DEBUG_PRINTLN(x,y);
        // return 0;
    }
};


void rpc_main(){
    uart2.init(576000);
    DEBUGGER.change(uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    Ball ball;

    auto res2 = rpc::make_rpc_memfunc<void, iq_t, iq_t>(ball, &Ball::set_xy);
    auto res = rpc::make_rpc_function<iq_t, iq_t, iq_t>(([](iq_t x, iq_t a){
        return sin(x + a);
    }));

    real_t a;
    auto p = rpc::make_property(a);

    // auto list = rpc::make_list("list", res, res2, p, rpc::make_property(a));
    auto list = rpc::make_list("list", res, res2, p);

    while(true){
        a = sin(8 * t);
        std::vector<rpc::CallParam> params = {
            rpc::CallParam(String(t)),
            rpc::CallParam(String(sin(t))),

            // rpc::MethodParam("1"),
            // rpc::MethodParam("2"),
            // rpc::MethodParam(String(millis())),
        };

        // res->call(DEBUGGER, params);
        // res2->call(DEBUGGER, params);
        // p->call(DEBUGGER, params);
        list->call(DEBUGGER, params);
        DEBUG_PRINTLN();
        // DEBUG_PRINTLN(params);
        delay(10);
    }
}   