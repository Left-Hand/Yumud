#pragma once

#include "rigid.hpp"

#include "core/stream/stream.hpp"
#include "core/stream/StringStream.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/rpc/arg_parser.hpp"

namespace nudec::_2023E{



class GimbalThread{
};

class GimbalThreadForMaster final: public GimbalThread{
public:
    void process(const real_t t){

    }
private:

};

class GimbalThreadForSlave final: public GimbalThread{
public:
    void process(const real_t t){

    }
private:

};

class ReplThread{
public:
    ReplThread(Uart & uart, rpc::EntryProxy && root) :
        uart_(uart), root_(std::move(root)){
            os_.retarget(&uart_);
        }
    void process(const real_t t){
        // if(uart_.available())DEBUG_PRINTLN(uart_.available());
        auto strs_opt = splitter_.update(uart_);
        if(strs_opt.has_value()){
            const auto & strs = strs_opt.value();

            if(outen_){
                os_.println("------");
                os_.prints("Inputs:", strs);
            }

            std::vector<rpc::CallParam> params;
            params.reserve(strs.size());
            for(const auto & str : strs){
                params.push_back(rpc::CallParam(str));
            }

            StringStream ss;
            const auto res = root_ ->call(ss, params);

            if(outen_){
                os_.print("->", std::move(ss).move_str());
                os_.prints("\r\n^^Function exited with return code", uint8_t(res));
                os_.println("------");
            }

            splitter_.clear();
        }
    }

    void set_outen(bool outen){ outen_ = outen; }   
private:
    Uart & uart_;
    OutputStreamByRoute os_;

    rpc::EntryProxy root_;
    ArgSplitter splitter_;

    bool outen_ = true;
};
}