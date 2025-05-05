#include "hal_result.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{

OutputStream & print_halerr_kind(OutputStream & os, const hal::HalError::Kind err){
    using Kind = hal::HalError::Kind;
    switch(err){
        case Kind::AlreadyUnderUse: return os << "AlreadyUnderUse";
        case Kind::OccuipedByOther: return os << "OccuipedByOther";
        case Kind::SlaveAddrAckTimeout: return os << "SlaveAddrAckTimeout";
        case Kind::WritePayloadAckTimeout: return os << "WritePayloadAckTimeout";
        case Kind::BusOverload: return os << "BusOverload";
        case Kind::SelecterOutOfRange: return os << "SelecterOutOfRange";
        case Kind::NoSelecter: return os << "NoSelecter";
        case Kind::PayloadNoLength: return os << "PayloadNoLength";
        case Kind::VerifyFailed: return os << "VerifyFailed";
        case Kind::LengthOverflow: return os << "LengthOverflow";
        case Kind::Unspecified: return os << "Unspecified";
        case Kind::TxLengthAndRxLengthNotMatch: return os << "TxLengthAndRxLengthNotMatch";
        case Kind::InvalidArgument: return os << "InvalidArgument";
    }
    return os;
}

OutputStream & operator << (OutputStream & os, const hal::HalResult & res){
    if(res.is_ok()) return os << "Ok";
    else{
        os << "Err(";
        print_halerr_kind(os, res.unwrap_err().kind());
        return os << ")" << os.endl();
    }
}

OutputStream & operator << (OutputStream & os, const hal::HalError & err){
    return print_halerr_kind(os, err.kind());
}

OutputStream & operator << (OutputStream & os, const hal::HalError::Kind & err_kind){
    return print_halerr_kind(os, err_kind);
}


}