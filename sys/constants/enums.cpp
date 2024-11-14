#include "enums.hpp"
#include "sys/stream/ostream.hpp"

using namespace ymd;

ymd::OutputStream & operator<<(ymd::OutputStream & os, const TB tb){
    // using namespace ymd::nvcv2;
    switch(tb){
        case TB::TOP: return os << 'T';break;
        case TB::BOTTOM: return os << 'B';break;
        default: return os << '?';break;
    };
}


ymd::OutputStream & operator<<(ymd::OutputStream & os, const LR lr){
    // using namespace ymd::nvcv2;
    switch(lr){
        case LR::LEFT: return os << 'L';break;
        case LR::RIGHT: return os << 'R';break;
        default: return os << '?';break;
    };
}