#include "enums.hpp"
#include "sys/stream/ostream.hpp"

OutputStream & operator<<(OutputStream & os, const TB tb){
    // using namespace NVCV2;
    switch(tb){
        case TB::TOP: return os << 'T';break;
        case TB::BOTTOM: return os << 'B';break;
        default: return os << '?';break;
    };
}


OutputStream & operator<<(OutputStream & os, const LR lr){
    // using namespace NVCV2;
    switch(lr){
        case LR::LEFT: return os << 'L';break;
        case LR::RIGHT: return os << 'R';break;
        default: return os << '?';break;
    };
}