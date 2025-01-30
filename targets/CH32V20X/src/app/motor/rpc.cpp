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

iq_t add(iq_t x, iq_t y){
    return x - y;
}


iq_t add2(const iq_t x, const iq_t y){
    return x - 0.9_r * y;
}

void rpc_main(){
    uart2.init(576000);
    DEBUGGER.change(uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    Ball ball;




    int v = 2;
    auto func = [&v](iq_t x, iq_t a){
        return sin(x) + a + v;
    };

    //function pointer
    auto res3 = rpc::make_function("add2", add2);

    //lambda
    auto res = rpc::make_function("add", func);

    //memfunc
    auto res2 = rpc::make_function("ball", ball, &Ball::set_xy);

    real_t a;
    const real_t b;
    auto p = rpc::make_property("a", a);

    auto list = rpc::make_list(
        "list", 
        res, 
        res2, 
        res3, 
        p,
        rpc::make_ro_property("b", b)
    );

    // char buf[64];
    // memset(buf, 0, sizeof(buf));
    // BufStream os(buf);
    // snprintf(os, 4,5,6,"Hello", iq_t(3.14));


    while(true){
        // a = sin(8 * t);
        // std::vector<rpc::CallParam> params = {
        //     rpc::CallParam(String(t)),
        //     rpc::CallParam(String(sin(t))),

        //     // rpc::MethodParam("1"),
        //     // rpc::MethodParam("2"),
        //     // rpc::MethodParam(String(millis())),
        // };

        // // res->call(DEBUGGER, params);
        // // res2->call(DEBUGGER, params);
        // // p->call(DEBUGGER, params);
        // // list->call(DEBUGGER, params);
        // // DEBUG_PRINTLN(buf);
        // // DEBUG_PRINTLN(params);
        // res->call(DEBUGGER, {{frac(t)}, {t}});
        // p->call(DEBUGGER, {});
        // DEBUG_PRINTLN("")
        // delay(10);
        // auto & body = *res;

        // res->call(DEBUGGER, rpc::Params{rpc::CallParam{t}, rpc::CallParam{t}});
        std::vector params_holder = {rpc::CallParam{t}, rpc::CallParam{t}};
        auto params = params_holder;
        // res->call(DEBUGGER, params);
        // res3 ->call(DEBUGGER, params);
        // list ->call(DEBUGGER, params);
        DEBUG_PRINTLN("")
        delay(1);
        // DEBUG_PRINTLN(res, sizeof(res));
    }
}   