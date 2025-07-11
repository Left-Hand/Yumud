#include "StringStream.hpp"

#if 0
using namespace ymd;


size_t StringStream::pending() const {
    return 0;
}

StringStream::operator String() const {
    return str_;
}

StringStream::operator StringView() const {
    return StringView(str_);
}
#endif