#pragma once

#include "core/math/real.hpp"
#include "core/string/own/heapless_string.hpp"
#include "core/string/legacy/streamed_string_splitter.hpp"

#include "middlewares/rpc/rpc.hpp"

namespace ymd::robots{

struct ReplServer final{
public:
    ReplServer(ReadCharProxy && is, WriteCharProxy && os) :
        is_(std::move(is)),
        os_(std::move(os)){;}

    template<typename T>
    void invoke(T && obj){
        while(is_->available()){
            uint8_t byte;
            is_->try_read_byte(byte);

            splitter_.update(byte,
                [this, &obj](const std::span<const StringView> strs){
                const auto res = respond(obj, strs);
                if(outen_){
                    os_.prints(">>=", res);
                }
            });
        }
    }

    void set_outen(Enable outen){ outen_ = outen == EN; }
private:
    ReadCharProxy is_;
    OutputStreamByRoute os_;

    StreamedStringSplitter splitter_;

    bool outen_ = false;

    template<typename T>
    auto respond(T && obj, const std::span<const StringView> strs){
        const auto guard = os_.create_guard();
        if(outen_){
            os_.force_sync(EN);
            os_.prints("<<=", strs);
        }

        return [&]{
            if(!this->outen_){
                // DummyOutputStream dos{};
                DummyReceiver dos{};

                return rpc::visit(obj, dos, rpc::AccessProvider_ByStringViews(strs));
            }else{
                return rpc::visit(obj, os_, rpc::AccessProvider_ByStringViews(strs));
            }
        }();
    }

    struct DummyReceiver{
        template<typename T>
        DummyReceiver & operator <<(T && arg){
            //do nothing
            return *this;
        }

        template<typename ... Args>
        void println(Args && ... args){;}
    };
};
}
