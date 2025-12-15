#pragma once

#include "core/string/string_view.hpp"
#include "core/utils/Option.hpp"
#include "core/string/heapless_string.hpp"
#include "core/container/heapless_vector.hpp"

namespace ymd{

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

}