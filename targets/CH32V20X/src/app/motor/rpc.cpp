#include "src/testbench/tb.h"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "robots/rpc/arg_parser.hpp"
#include "types/vector3/vector3.hpp"
#include "core/math/realmath.hpp"
#include "robots/rpc/rpc.hpp"

#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

class Ball{
public:
    int a;
    void set_xy(iq_t<16> x, iq_t<16> y){
        DEBUG_PRINTLN(x,y);
        // return 0;
    }
};

iq_t<16> add(iq_t<16> x, iq_t<16> y){
    return x - y;
}


iq_t<16> add2(const iq_t<16> x, const iq_t<16> y){
    return x - 0.9_r * y;
}

void rpc_main(){
    hal::uart2.init(576000);
    DEBUGGER.retarget(&hal::uart2);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");

    Ball ball;

    int v = 2;
    auto func = [&v](iq_t<16> x, iq_t<16> a){
        return sin(x) + a + v;
    };

    //function pointer
    auto res3 = rpc::make_function("add2", add2);


    //memfunc
    auto res2 = rpc::make_function("ball", &ball, &Ball::set_xy);

    real_t a;
    real_t b;
    auto p = rpc::make_property("a", a);

    hal::portA[8].outpp();

    auto list = rpc::make_list(
        "list", 
        rpc::make_function("add", func), 
        res2, 
        res3, 
        p,
        rpc::make_ro_property("b", b),
        rpc::make_ro_property("ba", ball.a),
        rpc::make_function("setm", [](const real_t duty){DEBUG_PRINTS("duty is set to:", duty);}),
        rpc::make_function("xyz", [](){DEBUG_PRINTLN(Vector3_t<real_t>(0,0,0));}),
        rpc::make_function("crc", [](){DEBUG_PRINTS(sys::chip::get_chip_id_crc());})
        // rpc::make_function("led", [](const int i){portA[8].write(i);})
    );

    ArgSplitter splitter;

    // pro::proxy_reflect
    // char buf[64];
    // memset(buf, 0, sizeof(buf));
    // BufStream os(buf);
    // snprintf(os, 4,5,6,"Hello", iq_t<16>(3.14));


    while(true){
        // a = sin(8 * t);
        // std::vector<rpc::CallParam> params = {
        //     rpc::CallParam(String(t)),
        //     rpc::CallParam(String(sin(t))),

        //     // rpc::MethodParam("1"),
        //     // rpc::MethodParam("2"),
        //     // rpc::MethodParam(String(millis())),
        // };


        continue;
        // // res->call(DEBUGGER, params);
        // // res2->call(DEBUGGER, params);
        // // p->call(DEBUGGER, params);
        // // list->call(DEBUGGER, params);
        // // DEBUG_PRINTLN(buf);
        // // DEBUG_PRINTLN(params);
        // res->call(DEBUGGER, {{frac(t)}, {t}});
        // p->call(DEBUGGER, {});
        // DEBUG_PRINTLN("")
        // clock::delay(10ms);
        // auto & body = *res;

        // res->call(DEBUGGER, rpc::Params{rpc::CallParam{t}, rpc::CallParam{t}});
        // std::vector params_holder = {rpc::CallParam{"b"}, rpc::CallParam{t}};

        clock::delay(1ms);
        // DEBUG_PRINTLN(res, sizeof(res));
    }
}   