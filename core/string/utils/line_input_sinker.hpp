#pragma once

#include "core/string/view/string_view.hpp"
#include "core/utils/scope_guard.hpp"
// #include "core/utils/iter/foreach.hpp"

namespace ymd{


struct [[nodiscard]] LineInputSinker final{

    constexpr explicit LineInputSinker(const std::span<char> buf){
        buf_ = buf;
        ind_ = 0;
    }

    std::span<char> buf_;
    size_t ind_;

    struct [[nodiscard]] PollResult final{
        using Self = PollResult;
        bool is_end_of_line;
        bool is_full;

        static constexpr Self from_default(){
            return Self{
                .is_end_of_line = false, 
                .is_full = false
            };
        }
    };

    constexpr PollResult push_char(const char chr){
        PollResult res = PollResult::from_default();
        if(ind_ >=  buf_.size()){
            res.is_full = true;
            return res;
        }

        if(ind_ != 0 and is_terminator(chr)){
            res.is_end_of_line = true;
        }

        buf_[ind_] = chr;
        ind_++;

        return res;
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept {
        return buf_.size();
    }

    constexpr StringView now_line(){
        return StringView{buf_.data(), ind_};
    } 

    constexpr void reset(){
        ind_ = 0;
    }

    constexpr StringView get_and_reset(){
        auto & self = *this;
        auto guard = make_scope_guard([&]{
            self.reset();
        });

        const auto line = now_line().trim();
        return line;
    }

private:
    static constexpr bool is_terminator(const char chr){
        return chr == '\r' || chr == '\n' || chr == '\0';
    }
};
}