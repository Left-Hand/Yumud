#include "can_enum.hpp"
#include "core/stream/ostream.hpp"

using namespace ymd;
namespace ymd::hal::can{
OutputStream & operator<<(OutputStream & os, const CanError & error){
    switch(error){
        case CanError::Stuff: 
            return os << "Stuff";
        case CanError::Form: 
            return os << "Form";
        case CanError::Acknowledge: 
            return os << "Acknowledge";
        case CanError::BitRecessive: 
            return os << "BitRecessive";
        case CanError::BitDominant: 
            return os << "BitDominant";
        case CanError::Crc: 
            return os << "Crc";
        case CanError::SoftwareSet:
            return os << "SoftwareSet";
    }
    __builtin_unreachable();
}

OutputStream & operator<<(OutputStream & os, const CanLibError & error){
    switch(error){
        case CanLibError::BlockingTransmitTimeout: 
            return os << "BlockingTransmitTimeout";
        case CanLibError::NoMailboxAvailable: 
            return os << "NoMailboxAvailable";
        case CanLibError::SoftFifoOverflow: 
            return os << "SoftFifoOverflow";
    }
    __builtin_unreachable();
}
}
