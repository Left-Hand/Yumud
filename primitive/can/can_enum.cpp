#include "can_enum.hpp"
#include "core/stream/ostream.hpp"

using namespace ymd;
namespace ymd::hal::can{
OutputStream & operator<<(OutputStream & os, const ErrorCode & error){
    switch(error){
        case ErrorCode::Ok:
            return os << "Ok";
        case ErrorCode::Stuff: 
            return os << "Stuff";
        case ErrorCode::Form: 
            return os << "Form";
        case ErrorCode::Acknowledge: 
            return os << "Acknowledge";
        case ErrorCode::BitRecessive: 
            return os << "BitRecessive";
        case ErrorCode::BitDominant: 
            return os << "BitDominant";
        case ErrorCode::Crc: 
            return os << "Crc";
        case ErrorCode::SoftwareSet:
            return os << "SoftwareSet";
    }
    __builtin_unreachable();
}


OutputStream & operator<<(OutputStream & os, const LibError & error){
    switch(error){
        case LibError::NoMailboxAvailable: 
            return os << "NoMailboxAvailable";
        case LibError::TxQueueFull: 
            return os << "TxQueueFull";
        case LibError::TxQueueOverflow: 
            return os << "TxQueueOverflow";
    }
    __builtin_unreachable();
}
}
