#include "uart_primitive.hpp"
#include "core/stream/ostream.hpp"
#include "core/tmp/reflect/enum.hpp"

namespace ymd::hal{

OutputStream & operator << (OutputStream & os, const UartEvent & event){
    DeriveDebugDispatcher<UartEvent::Kind>::call(os, event.kind());
    return os;
}
}