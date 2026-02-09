#include "core/debug/debug.hpp"
#include "asciican_primitive.hpp"

using namespace ymd;
using namespace ymd::asciican;
using namespace ymd::asciican::primitive;


template<typename T = void>
using IResult = Result<T, Error>;


namespace ymd::asciican{
::ymd::OutputStream& operator<<(::ymd::OutputStream& os,const Error & value){
    DeriveDebugDispatcher<Error>::call(os, value);
    return os;
}
}