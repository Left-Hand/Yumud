#include "StringStream.hpp"


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