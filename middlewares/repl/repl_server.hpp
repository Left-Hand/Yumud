#pragma once

#include "core/math/real.hpp"
#include "core/string/owned/heapless_string.hpp"
#include "core/container/heapless_vector.hpp"
#include "middlewares/repl/repl.hpp"
#include "core/string/utils/line_input_sinker.hpp"
#include "core/string/utils/split_iter.hpp"
#include "core/utils/iter/foreach.hpp"

namespace ymd::repl{

struct [[nodiscard]] ReplServer final{
public:
    ReplServer(ReadCharProxy && is, WriteCharProxy && os) :
        is_(std::move(is)),
        os_(std::move(os)){;}

    template<typename T>
    void invoke(T && obj){
        if(not is_->available()) return;
        while(true){
            uint8_t byte;
            if(const auto len = is_->try_read_byte(byte);
                len == 0) break;
            const auto res = line_sinker_.push_char(static_cast<char>(byte));
            if(res.is_full) line_sinker_.reset();
            if(res.is_end_of_line){
                // DEBUG_PRINTLN(line_sinker_.dump_and_reset());
                auto line = line_sinker_.get_and_reset();
                auto iter = StringSplitIter(line, ' ') | foreach([](StringView token){ 
                    return token.trim();
                });
                const auto strs = [&]{
                    HeaplessVector<StringView, 12> ret;
                    while(iter.has_next()){
                        if(ret.append(iter.next()).is_err()) break; 
                    }
                    return ret;
                }();

                const auto resp_res = respond(obj, strs);
                if(echo_en_){
                    os_.prints(StringView(">>="), resp_res);
                }
        

            }
        }
    }

    void enable_echo(Enable echo_en){ echo_en_ = echo_en == EN; }
// private:
public:
    ReadCharProxy is_;
    OutputStreamByRoute os_;


    std::array<char, 64> str_buf_;
    LineInputSinker line_sinker_ = LineInputSinker{std::span(str_buf_)};

    bool echo_en_ = false;

    template<typename T>
    auto respond(T && obj, const std::span<const StringView> strs){
        const auto guard = os_.create_guard();
        if(echo_en_){
            os_.force_sync(EN);
            os_ << StringView("<<= ") << strs;
        }

        os_ << StringView("\r\n----\r\n");

        return [&]{
            if(!this->echo_en_){
                // DummyOutputStream dos{};
                DummyReceiver dos{};

                return script::visit(obj, dos, script::AccessProvider_ByStringViews(strs));
            }else{
                auto t_guard = make_scope_guard([&]{
                    os_ << StringView("\r\n----\r\n");
                });
                
                return script::visit(obj, os_, script::AccessProvider_ByStringViews(strs));
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
