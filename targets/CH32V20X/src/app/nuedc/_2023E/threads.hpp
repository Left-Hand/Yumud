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
        while(uart_.available()){
            char chr;
            uart_.read1(chr);
            splitter_.update(chr, [this](const StringViews strs){
                if(outen_){
                    os_.println("------");
                    os_.prints("Inputs:", strs);
                }
    
                // StringStream ss;
                const auto res = root_ ->call(os_, rpc::AccessProvider_ByStringViews(strs));
    
                if(outen_){
                    // os_.print("->", std::move(ss).move_str());
                    os_.prints("\r\n^^Function exited with return code", uint8_t(res));
                    os_.println("------");
                }
            });
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