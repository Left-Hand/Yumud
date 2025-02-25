#include "rpc.hpp"
#include "tb.h"
#include "sys/debug/debug_inc.h"
#include "robots/rpc/arg_parser.hpp"
#include "types/vector3/vector3_t.hpp"
#include "sys/clock/time.hpp"

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
    uart2.init(576000);
    DEBUGGER.retarget(uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");

    Ball ball;

    int v = 2;
    auto func = [&v](iq_t<16> x, iq_t<16> a){
        return sin(x) + a + v;
    };

    //function pointer
    auto res3 = rpc::make_function("add2", add2);


    //memfunc
    auto res2 = rpc::make_function("ball", ball, &Ball::set_xy);

    real_t a;
    real_t b;
    auto p = rpc::make_property("a", a);

    portA[8].outpp();

    auto list = rpc::make_list(
        "list", 
        rpc::make_function("add", func), 
        res2, 
        res3, 
        p,
        rpc::make_ro_property("b", b),
        rpc::make_ro_property("ba", ball.a),
        rpc::make_function("setm", [](const real_t duty){DEBUG_PRINTS("duty is set to:", duty)}),
        rpc::make_function("xyz", [](){DEBUG_PRINTLN(Vector3_t<real_t>(0,0,0))}),
        rpc::make_function("crc", [](){DEBUG_PRINTS(sys::Chip::getChipIdCrc())}),
        rpc::make_function("led", [](const int i){portA[8].write(i);})
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
        {
            auto strs_opt = splitter.update(uart2);
            if(strs_opt.has_value()){
                auto & strs = strs_opt.value();

                DEBUG_PRINTLN("------");
                DEBUG_PRINTS("Inputs:", strs);

                b = sin(time());

                {
                    std::vector<rpc::CallParam> params;
                    params.reserve(strs.size());
                    for(const auto & str  : strs){
                        params.push_back(rpc::CallParam(str));
                    }
                    DEBUGGER.print("->");
                    auto res = list ->call(DEBUGGER, params);
                    DEBUG_PRINTS("\r\n^^Function exited with return code", uint8_t(res))
                    DEBUG_PRINTLN("------");
                }

                splitter.clear();
            }
        }

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
        // delay(10);
        // auto & body = *res;

        // res->call(DEBUGGER, rpc::Params{rpc::CallParam{t}, rpc::CallParam{t}});
        // std::vector params_holder = {rpc::CallParam{"b"}, rpc::CallParam{t}};

        delay(1);
        // DEBUG_PRINTLN(res, sizeof(res));
    }
}   