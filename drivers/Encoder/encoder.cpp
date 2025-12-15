#include "Encoder.hpp"
#include "core/tmp/reflect/enum.hpp"

using namespace ymd;
using namespace ymd::drivers;

namespace ymd{
OutputStream& operator<<(OutputStream& os, const drivers::details::EncoderError_Kind value){
    DeriveDebugDispatcher<drivers::details::EncoderError_Kind>::call(os, value);
    return os;
}
}
