#include "uart_primitive.hpp"
#include "core/stream/ostream.hpp"
#include "core/magic/enum_traits.hpp"

namespace ymd::hal{

OutputStream & operator << (OutputStream & os, const UartEvent & event){
    DeriveDebugDispatcher<UartEvent::Kind>::call(os, event.kind());
    return os;
}
}