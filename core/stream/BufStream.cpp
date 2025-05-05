#include "BufStream.hpp"

using namespace ymd;



BufStream::operator String() const {
    return buf_;
}

BufStream::operator StringView() const {
    return StringView(buf_);
}