#include "keycode.hpp"


namespace ymd::hid{
::ymd::OutputStream & operator <<(::ymd::OutputStream & os, const hid::KeyCode_Kind kind){
    using Kind = hid::KeyCode_Kind;
    const auto guard = os.create_guard();
    os << std::dec;
    switch(kind){
        default: 
            os << "Unprintable Key";
            break;
        case Kind::Digit0 ... Kind::Digit9: 
            os << char((uint8_t(kind) - uint8_t(Kind::Digit0)) + '0');
            break;
        case Kind::Numpad0 ... Kind::Numpad9: 
            os << char((uint8_t(kind) - uint8_t(Kind::Numpad0)) + '0');
            break;
        case Kind::F1 ... Kind::F35: 
            os << 'F' << char(uint8_t(kind) - uint8_t(Kind::F1));
            break;
        case Kind::KeyA ... Kind::KeyZ:
            os << char((uint8_t(kind) - uint8_t(Kind::KeyA)) + 'A');
            break;

        case Kind::NumpadSubtract:
            os << '-';
            break;
        case Kind::NumpadAdd:
            os << '+';
            break;
        case Kind::Comma:
        case Kind::NumpadComma:
            os << '.';
            break;
        case Kind::ArrowLeft:
            os << "ArrowLeft";
            break;
        case Kind::ArrowRight:
            os << "ArrowRight";
            break;
        case Kind::ArrowUp:
            os << "ArrowUp";
            break;
        case Kind::ArrowDown:
            os << "ArrowDown";
            break;
    }

    return os;
}

}