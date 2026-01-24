#pragma once

#include "core/math/real.hpp"
#include "core/string/owned/heapless_string.hpp"
#include "core/container/heapless_vector.hpp"
#include "middlewares/repl/repl.hpp"

namespace ymd::repl{

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



    struct StreamedStringSplitter final{
    public:
        constexpr StreamedStringSplitter(){;}

        template<typename Fn>
        constexpr void update(const char chr, Fn && fn){
            if(is_valid_char(chr)){
                temp_str_.push_back_unchecked(chr);
            }

            if(temp_str_.size() >= STR_MAX_LENGTH){
                on_buf_overflow();
                temp_str_.clear();
            }

            if(chr == '\n'){
                send_line(
                    StringView(temp_str_.data(), temp_str_.size()), 
                    delimiter_,
                    std::forward<Fn>(fn)
                );

                temp_str_.clear();
            }
        }

        constexpr StringView temp() const {
            return StringView{temp_str_.data(), temp_str_.size()};
        }

    private:
        static constexpr size_t STR_MAX_LENGTH = 64;
        static constexpr size_t STR_MAX_PIECES = 8;

        HeaplessString<STR_MAX_LENGTH> temp_str_;
        char delimiter_ = ' ';

        template<typename Fn>
        static constexpr void send_line(const StringView line, const char delimiter, Fn && fn) {

            HeaplessVector<StringView, STR_MAX_PIECES> str_pieces;
        
            size_t from = 0;
            for(size_t i = 0; i < line.length(); i++){
                if((i == 0 || line[i - 1] == delimiter) and line[i] != delimiter){
                    from = i;
                }
        
                
                if(line[i] != delimiter and 
                    (i + 1 >= line.length() or line[i + 1] == delimiter or line[i + 1] == '\0')){
                    str_pieces.push_back(line.substr_by_range(from, i+1).unwrap());
                }
        
                if((str_pieces.size() == STR_MAX_PIECES)){
                    on_pieces_overflow();
                    return;
                }
            }
        
            std::forward<Fn>(fn)(std::span<StringView>(str_pieces.data(), str_pieces.size()));
        }

        static constexpr bool is_valid_char(const char c){
            return (c >= 32) and (c <= 126);
        }

        static constexpr void on_buf_overflow(){
            //TODO
        }

        static constexpr void on_pieces_overflow(){
            //TODO
        }

    };

    
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

                return script::visit(obj, dos, script::AccessProvider_ByStringViews(strs));
            }else{
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
