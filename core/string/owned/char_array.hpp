#pragma once

#include "core/string/view/string_view.hpp"
#include "core/stream/ostream.hpp"

#include "core/utils/Result.hpp"


namespace ymd{

template<size_t N>
// using CharArray = std::array<char, N>;
struct [[nodiscard]] CharArray final{
    using Self = CharArray;
    std::array<char, N> chars;

    constexpr CharArray(){;}
    constexpr CharArray(const char (&str)[N]){
        *this = CharArray(std::span<const char, N>(str));
    }

    //panics: chars length overflow
    constexpr CharArray(const std::span<const char> _chars){
        if(_chars.size() > N)
            __builtin_trap();

        for(size_t i = 0; i < _chars.size(); i++){
            chars[i] = _chars[i];
        }
    }

    static constexpr CharArray from_chars_bounded(std::span<const char> chars){
        Self self;

        const size_t len = std::min(chars.size(), N);

        for(size_t i = 0; i < len; i++){
            self.chars[i] = chars[i];
        }

        return self;
    }

    constexpr CharArray(const StringView str){
        for(size_t i = 0; i < N; i++){
            chars[i] = str[i];
        }
    }

    [[nodiscard]] static consteval size_t capacity(){
        return N;
    }

    [[nodiscard]] constexpr size_t length() const {
        return strnlen(chars.data(), N);
    }

    [[nodiscard]] constexpr const char * data() const {
        return chars.data();
    }

    constexpr void fill(const char c){
        std::fill(chars.begin(), chars.end(), c);
    }

    constexpr char operator[](size_t i) const {
        return chars[i];
    }

    constexpr char & operator[](size_t i) {
        return chars[i];
    }
};

//ctad from c-style char array
template<size_t N>
CharArray(const char (&)[N]) -> CharArray<N>;
}