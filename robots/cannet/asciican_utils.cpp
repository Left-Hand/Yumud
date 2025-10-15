#include "asciican_utils.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::robots::asciican;

using Error = AsciiCanError;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> AsciiCanPhy::send_can_msg(const Msg && msg){
    // DEBUG_PRINTLN("send can msg", msg);
    can_.write(msg).examine();
    return Ok();
}

IResult<> AsciiCanPhy::send_str(const StringView str){
    // DEBUG_PRINTLN("send str", str);
    DEBUG_PRINTLN(str);
    return Ok();
}

IResult<> AsciiCanPhy::set_stream_baud(const uint32_t baud){
    DEBUG_PRINTLN("set_stream_baud", baud);
    return Ok();
}

IResult<> AsciiCanPhy::set_can_baud(const uint32_t baud){
    DEBUG_PRINTLN("set_can_baud", baud);
    return Ok();
}

IResult<> AsciiCanPhy::open(){
    DEBUG_PRINTLN("open");
    return Ok();
}

IResult<> AsciiCanPhy::close(){
    DEBUG_PRINTLN("close");
    return Ok();
}

namespace ymd::robots::asciican{
::ymd::OutputStream& operator<<(::ymd::OutputStream& os,const AsciiCanError & value){
    DeriveDebugDispatcher<AsciiCanError>::call(os, value);
    return os;
}
}