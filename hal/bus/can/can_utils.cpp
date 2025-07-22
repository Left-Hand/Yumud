#include "can_utils.hpp"
#include "core/stream/ostream.hpp"

using namespace ymd;
namespace ymd::hal{
OutputStream & operator<<(OutputStream & os, const CanFault & fault){
    switch(fault){
        case CanFault::Stuff: 
            return os << "Stuff";
        case CanFault::Form: 
            return os << "Form";
        case CanFault::Acknowledge: 
            return os << "Acknowledge";
        case CanFault::BitRecessive: 
            return os << "BitRecessive";
        case CanFault::BitDominant: 
            return os << "BitDominant";
        case CanFault::Crc: 
            return os << "Crc";
        default: __builtin_unreachable();
    }
}

OutputStream & operator<<(OutputStream & os, const CanError & error){
    switch(error){
        case CanError::BlockingTransmitTimeout: 
            return os << "BlockingTransmitTimeout";
        case CanError::NoMailboxFounded: 
            return os << "NoMailboxFounded";
        case CanError::SoftFifoOverflow: 
            return os << "SoftFifoOverflow";
        default: __builtin_unreachable();
    }
}
}