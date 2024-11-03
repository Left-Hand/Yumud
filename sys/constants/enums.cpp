#include "enums.hpp"
#include "sys/stream/ostream.hpp"

using namespace yumud;

yumud::OutputStream & operator<<(yumud::OutputStream & os, const TB tb){
    // using namespace yumud::nvcv2;
    switch(tb){
        case TB::TOP: return os << 'T';break;
        case TB::BOTTOM: return os << 'B';break;
        default: return os << '?';break;
    };
}


yumud::OutputStream & operator<<(yumud::OutputStream & os, const LR lr){
    // using namespace yumud::nvcv2;
    switch(lr){
        case LR::LEFT: return os << 'L';break;
        case LR::RIGHT: return os << 'R';break;
        default: return os << '?';break;
    };
}