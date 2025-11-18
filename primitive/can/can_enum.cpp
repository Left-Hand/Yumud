#include "can_enum.hpp"
#include "core/stream/ostream.hpp"

using namespace ymd;
namespace ymd::hal{
OutputStream & operator<<(OutputStream & os, const CanException & exception){
    switch(exception){
        case CanException::Stuff: 
            return os << "Stuff";
        case CanException::Form: 
            return os << "Form";
        case CanException::Acknowledge: 
            return os << "Acknowledge";
        case CanException::BitRecessive: 
            return os << "BitRecessive";
        case CanException::BitDominant: 
            return os << "BitDominant";
        case CanException::Crc: 
            return os << "Crc";
        case CanException::SoftwareSet:
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