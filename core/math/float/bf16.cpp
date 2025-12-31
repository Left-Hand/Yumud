#include "bf16.hpp"
#include "fp32.hpp"
#include "core/stream/ostream.hpp"

namespace ymd{
OutputStream & operator << (OutputStream & os, const math::bf16 f_val){
    return os << static_cast<float>(f_val);
    // return os << os.scoped("bf16")(
    //     os << os.field("frac")(f_val.frac) 
    //     << os.field("exp")(f_val.exp) 
    //     << os.field("sign")(f_val.sign)
    // );
}

OutputStream & operator << (OutputStream & os, const math::fp32 f_val){
    return os << static_cast<float>(f_val);
    // return os << iq16::from(static_cast<float>(f_val)) << os.splitter()
    //     << std::hex << std::showbase << f_val.to_bits();
    // return os << os.scoped("bf16")(
    //     os << os.field("frac")(f_val.frac) 
    //     << os.field("exp")(f_val.exp) 
    //     << os.field("sign")(f_val.sign)
    // );
}



}

#if 0
namespace {
void test1(){
    {
        constexpr auto f = std::bit_cast<float>(0xE51386C2);
    }
    {
        constexpr auto f = std::bit_cast<float>(0x928A6A43);
    }
}
}
#endif