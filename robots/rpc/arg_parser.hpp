#pragma once

#include "core/string/string_view.hpp"
#include "core/utils/Option.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "thirdparty/sstl/include/sstl/vector.h"


namespace ymd{

struct ArgSplitter final{

public:
    ArgSplitter(){;}

    template<typename Fn>
    constexpr void update(const char chr, Fn && fn){
        if(is_valid_char(chr)){
            temp_str_.push_back(chr);
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

    StringView temp() const {
        return StringView{temp_str_.data(), temp_str_.size()};
    }

private:
    static constexpr size_t STR_MAX_LENGTH = 64;
    static constexpr size_t STR_MAX_PIECES = 16;

    sstl::vector<char, STR_MAX_LENGTH> temp_str_;
    char delimiter_ = ' ';

    template<typename Fn>
    static constexpr void send_line(const StringView line, const char delimiter, Fn && fn) {

        sstl::vector<StringView, STR_MAX_LENGTH> strs;
    
        size_t from = 0;
        for(size_t i = 0; i < line.length(); i++){
            if((i == 0 || line[i - 1] == delimiter) and line[i] != delimiter){
                from = i;
            }
    
            
            if(line[i] != delimiter and 
                (i + 1 >= line.length() or line[i + 1] == delimiter or line[i + 1] == '\0')){
                strs.push_back(line.substr_by_range(from, i+1));
            }
    
            if((strs.size() == STR_MAX_PIECES)){
                on_pieces_overflow();
                return;
            }
        }
    
        std::forward<Fn>(fn)(std::span<StringView>(strs.data(), strs.size()));
    }

    static constexpr bool is_valid_char(const char c){
        switch(c){
            case ' ': return true;
            case '0'...'9': return true;
            case 'a'...'z': return true;
            case 'A'...'Z': return true;
            case '.': return true;
            case '-': return true;
            case '_': return true;
            case '+': return true;
            default: return false;
        }
    }

    static constexpr void on_buf_overflow(){
        //TODO
    }

    static constexpr void on_pieces_overflow(){
        //TODO
    }

};

}