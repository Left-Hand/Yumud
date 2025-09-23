#include "keycode.hpp"


namespace ymd::hid{
::ymd::OutputStream & operator <<(
    ::ymd::OutputStream & os, 
    const hid::KeyCode_Kind kind
){
    using Kind = hid::KeyCode_Kind;
    const auto guard = os.create_guard();
    os << std::dec << os.brackets<'['>();

    [[maybe_unused]]auto & os_ = [&] -> OutputStream & {
        switch(kind){
            default: 
                return os << "Unprintable Key";
            case Kind::Digit0 ... Kind::Digit9: 
                return os << char((uint8_t(kind) - uint8_t(Kind::Digit0)) + '0');
            case Kind::Numpad0 ... Kind::Numpad9: 
                return os << char((uint8_t(kind) - uint8_t(Kind::Numpad0)) + '0');
            case Kind::F1 ... Kind::F35: 
                return os << 'F' << char(uint8_t(kind) - uint8_t(Kind::F1));
            case Kind::KeyA ... Kind::KeyZ:
                return os << char((uint8_t(kind) - uint8_t(Kind::KeyA)) + 'A');
            case Kind::NumpadSubtract:
                return os << '-';
            case Kind::NumpadAdd:
                return os << '+';
            case Kind::Comma:
            case Kind::NumpadComma:
                return os << '.';
            case Kind::ArrowLeft:
                return os << "ArrowLeft";
            case Kind::ArrowRight:
                return os << "ArrowRight";
            case Kind::ArrowUp:
                return os << "ArrowUp";
            case Kind::ArrowDown:
                return os << "ArrowDown";
            case Kind::NumpadBackspace:
            case Kind::Backspace:
                return os << "Backspace";
            case Kind::Enter:
            case Kind::NumpadEnter:
                return os << "Enter";
            case Kind::NumpadClear:
                return os << "Clear";
            case Kind::Delete:
                return os << "Delete";
        }
        __builtin_unreachable();
    }();

    os << os.brackets<'('>() << std::bit_cast<uint8_t>(kind) << os.brackets<')'>();
    return os << os.brackets<']'>();
}

}