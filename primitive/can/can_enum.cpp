#include "can_enum.hpp"
#include "core/stream/ostream.hpp"

using namespace ymd;
namespace ymd::hal::can{
OutputStream & operator<<(OutputStream & os, const Error & error){
    switch(error){
        case Error::Stuff: 
            return os << "Stuff";
        case Error::Form: 
            return os << "Form";
        case Error::Acknowledge: 
            return os << "Acknowledge";
        case Error::BitRecessive: 
            return os << "BitRecessive";
        case Error::BitDominant: 
            return os << "BitDominant";
        case Error::Crc: 
            return os << "Crc";
        case Error::SoftwareSet:
            return os << "SoftwareSet";
    }
    __builtin_unreachable();
}


OutputStream & operator<<(OutputStream & os, const LibError & error){
    switch(error){
        case LibError::NoMailboxAvailable: 
            return os << "NoMailboxAvailable";
        case LibError::SoftQueueFull: 
            return os << "SoftQueueFull";
    }
    __builtin_unreachable();
}
}
