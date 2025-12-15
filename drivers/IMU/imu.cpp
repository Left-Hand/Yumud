#include "imu.hpp"
#include "core/tmp/reflect/enum.hpp"

using namespace ymd;
using namespace ymd::drivers;

namespace ymd{
OutputStream& operator<<(OutputStream& os, const drivers::details::ImuError_Kind value){
    DeriveDebugDispatcher<drivers::details::ImuError_Kind>::call(os, value);
    return os;
}
}
