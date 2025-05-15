#include "hal_result.hpp"
#include "core/stream/ostream.hpp"
#include "core/utils/typetraits/enum_traits.hpp"

namespace ymd{

OutputStream & print_halerr_kind(OutputStream & os, const hal::HalError::Kind err){
    derive_debug_dispatcher<hal::HalError::Kind>::call(os, err);
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