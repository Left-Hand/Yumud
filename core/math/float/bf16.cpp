#include "bf16.hpp"
#include "fp32.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{
OutputStream & operator << (OutputStream & os, const math::bf16 f_val){
    return os << static_cast<float>(f_val);
    // return os << os.scoped("bf16")(
    //     os << os.field("frac")(f_val.raw.frac) 
    //     << os.field("exp")(f_val.raw.exp) 
    //     << os.field("sign")(f_val.raw.sign)
    // );
}

OutputStream & operator << (OutputStream & os, const math::fp32 f_val){
    return os << iq16::from(static_cast<float>(f_val));
    // return os << os.scoped("bf16")(
    //     os << os.field("frac")(f_val.raw.frac) 
    //     << os.field("exp")(f_val.raw.exp) 
    //     << os.field("sign")(f_val.raw.sign)
    // );
}



}