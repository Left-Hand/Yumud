#include "hal_result.hpp"
#include "core/stream/ostream.hpp"
#include "core/utils/typetraits/enum_traits.hpp"

namespace ymd{

OutputStream & print_halerr_kind(OutputStream & os, const hal::HalError::Kind err){
    DeriveDebugDispatcher<hal::HalError::Kind>::call(os, err);
    return os;
}

OutputStream & operator << (OutputStream & os, const hal::HalResult & res){
    if(res.is_ok())
        os << "Ok";
    else{
        os << "Err" << os.brackets<'('>();
        print_halerr_kind(os, res.unwrap_err().kind());
        os << os.brackets<')'>();
    }
    return os;
}

OutputStream & operator << (OutputStream & os, const hal::HalError & err){
    return print_halerr_kind(os, err.kind());
}

OutputStream & operator << (OutputStream & os, const hal::HalError::Kind & err_kind){
    return print_halerr_kind(os, err_kind);
}


}