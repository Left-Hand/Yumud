#include "BufStream.hpp"

using namespace ymd;


BufStream::operator StringView() const {
    return StringView(buf_, len_);
}