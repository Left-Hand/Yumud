#pragma once

#include "core/utils/Option.hpp"

namespace ymd::hid{

struct [[nodiscard]] SegCode{

    enum class [[nodiscard]] SegCode_Kind:uint8_t{
        _0 = 0x3F,  //"0"
        _1 = 0x06,  //"1"
        _2 = 0x5B,  //"2"
        _3 = 0x4F,  //"3"
        _4 = 0x66,  //"4"
        _5 = 0x6D,  //"5"
        _6 = 0x7D,  //"6"
        _7 = 0x07,  //"7"
        _8 = 0x7F,  //"8"
        _9 = 0x6F,  //"9"
        A = 0x77,  //"A"
        B = 0x7C,  //"B"
        C = 0x39,  //"C"
        D = 0x5E,  //"D"
        E = 0x79,  //"E"
        F = 0x71,  //"F"
        H = 0x76,  //"H"
        L = 0x38,  //"L"
        n = 0x37,  //"n"
        u = 0x3E,  //"u"
        P = 0x73,  //"P"
        O = 0x5C,  //"o"
        _ = 0x40,  //"-"
        Dot = 0x80,
        Off = 0x00  //熄灭
    };

    using Kind = SegCode_Kind;
    using enum Kind;

    constexpr SegCode(Kind kind):kind_(kind){}

    static constexpr Option<SegCode> from_char(char chr){
        switch(chr){
            default: return None;
            case '0': return Some(_0);
            case '1': return Some(_1);
            case '2': return Some(_2);
            case '3': return Some(_3);
            case '4': return Some(_4);
            case '5': return Some(_5);
            case '6': return Some(_6);
            case '7': return Some(_7);

            case '8': return Some(_8);
            case '9': return Some(_9);
            case 'A': return Some(A);
            case 'B': return Some(B);
            case 'C': return Some(C);
            case 'D': return Some(D);
            case 'E': return Some(E);

            case '-': return Some(_);
            case '.': return Some(Dot);

            case 0: return Some(Off);
        }
    }

    [[nodiscard]] constexpr Kind kind() const{return kind_;}

    [[nodiscard]] constexpr bool operator==(Kind kind) const{return kind == this->kind_;}
    [[nodiscard]] constexpr bool operator!=(Kind kind) const{return kind != this->kind_;}

private:
    Kind kind_;
};

}