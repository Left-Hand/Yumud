#include "bf16.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{
OutputStream & operator << (OutputStream & os, const math::bf16 f_val){
    // uint16_t frac:7;
    // uint16_t exp:8;
    // uint16_t sign:1;
    // return os << float(1.0);
    // return os << fixed_t<16, int32_t>::from(float(f_val));
    // return os.operator<<(float(f_val));
    return os << static_cast<float>(f_val);
    // return os << os.scoped("bf16")(
    //     os << os.field("frac")(f_val.raw.frac) 
    //     << os.field("exp")(f_val.raw.exp) 
    //     << os.field("sign")(f_val.raw.sign) << os.endl()
    // );
}

}