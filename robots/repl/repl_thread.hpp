#pragma once

#include "core/math/real.hpp"
#include "core/stream/dummy_stream.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/rpc/arg_parser.hpp"

namespace ymd::robots{
class ReplThread{
public:
    ReplThread(ReadCharProxy && is, WriteCharProxy && os, rpc::EntryProxy && root) :
        is_(std::move(is)), os_(std::move(os)), root_(std::move(root)){}
    void process(const real_t t){
        while(is_->available()){
            char chr;
            is_->read1(chr);
            splitter_.update(chr, [this](const StringViews strs){
                if(outen_){
                    os_.println("------");
                    os_.prints("Inputs:", strs);
                }

                const auto res = [&]{
                    if(!this->outen_){
                        DummyOutputStream dos{};
                        return root_ ->call(dos, rpc::AccessProvider_ByStringViews(strs));
                    }else return root_ ->call(os_, rpc::AccessProvider_ByStringViews(strs));
                }();
    
                if(outen_){
                    os_.prints("\r\n^^Function exited with return code", uint8_t(res));
                    os_.println("------");
                }
            });
        }

    }

    void set_outen(bool outen){ outen_ = outen; }   
private:
    ReadCharProxy is_;
    OutputStreamByRoute os_;

    rpc::EntryProxy root_;
    ArgSplitter splitter_;

    bool outen_ = true;
};
}