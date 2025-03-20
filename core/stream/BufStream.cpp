#include "BufStream.hpp"

using namespace ymd;


void BufStream::write(const char data){
    buf_[len_ ++] = data;
}

void BufStream::write(const char * data_ptr, const size_t len) {
    memcpy(buf_ + len_, data_ptr, len);
    len_ += len;
}


BufStream::operator String() const {
    return buf_;
}

BufStream::operator StringView() const {
    return StringView(buf_);
}