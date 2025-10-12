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
        case CanFault::SoftwareSet:
            return os << "SoftwareSet";
    }
    __builtin_unreachable();
}

OutputStream & operator<<(OutputStream & os, const CanError & error){
    switch(error){
        case CanError::BlockingTransmitTimeout: 
            return os << "BlockingTransmitTimeout";
        case CanError::NoMailboxAvailable: 
            return os << "NoMailboxAvailable";
        case CanError::SoftFifoOverflow: 
            return os << "SoftFifoOverflow";
    }
    __builtin_unreachable();
}
}