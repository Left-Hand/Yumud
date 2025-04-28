#include "bus_error.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{



OutputStream & print_buserr_kind(OutputStream & os, const hal::HalError::Kind err){
    using Kind = hal::HalError::Kind;
    switch(err){
        case Kind::AlreadyUnderUse: return os << "AlreadyUnderUse";
        case Kind::OccuipedByOther: return os << "OccuipedByOther";
        case Kind::AckTimeout: return os << "AckTimeout";
        case Kind::BusOverload: return os << "BusOverload";
        case Kind::SelecterOutOfRange: return os << "SelecterOutOfRange";
        case Kind::NoSelecter: return os << "NoSelecter";
        case Kind::PayloadNoLength: return os << "PayloadNoLength";
        case Kind::VerifyFailed: return os << "VerifyFailed";
        case Kind::LengthOverflow: return os << "LengthOverflow";
        case Kind::Unspecified: return os << "Unspecified";
        default: return os << "Unknown";
    }
    return os;
}

OutputStream & operator << (OutputStream & os, const hal::HalResult & res){
    if(res.is_ok()) return os << "Ok";
    else return print_buserr_kind(os, res.unwrap_err().kind());
}

OutputStream & operator << (OutputStream & os, const hal::HalError & err){
    return print_buserr_kind(os, err.kind());
}

OutputStream & operator << (OutputStream & os, const hal::HalError::Kind & err_kind){
    return print_buserr_kind(os, err_kind);
}
}