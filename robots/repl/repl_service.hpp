#pragma once

#include "core/math/real.hpp"
#include "core/stream/dummy_stream.hpp"
#include "core/string/fixed_string.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "robots/rpc/rpc.hpp"
#include "robots/rpc/arg_parser.hpp"

namespace ymd::robots{

struct ReplServer final{
public:
    ReplServer(ReadCharProxy && is, WriteCharProxy && os) :
        is_(std::move(is)), 
        os_(std::move(os)){;}

    template<typename T>
    void invoke(T && obj){
        while(is_->available()){
            char chr;
            is_->read1(chr);

            // temp_str_.push_back(chr).unwrap();

            // if(chr == '\n'){
            //     // DEBUG_PRINTLN(splitter_.temp());
            //     DEBUG_PRINTLN(temp_str_);
            //     temp_str_.clear();
            // }

            splitter_.update(chr, [this, &obj](const std::span<const StringView> strs){
                respond(obj, strs);});
        }
    }

    void set_outen(bool outen){ outen_ = outen; }   
private:
    ReadCharProxy is_;
    OutputStreamByRoute os_;

    ArgSplitter splitter_;
    // FixedString<64> temp_str_;
    
    bool outen_ = true;
    
    template<typename T>
    void respond(T && obj, const std::span<const StringView> strs){
        const auto guard = os_.create_guard();
        os_.force_sync();
        if(outen_){
            os_.prints("<<=", strs);
        }

        const auto res = [&]{
            if(!this->outen_){
                DummyOutputStream dos{};
                return rpc::visit(obj, dos, rpc::AccessProvider_ByStringViews(strs));
            }else{
                return rpc::visit(obj, os_, rpc::AccessProvider_ByStringViews(strs));
            }
        }();

        
        if(outen_){
            os_.prints(">>=", res);
        }
    }
};
}