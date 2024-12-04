#include "StringStream.hpp"


using namespace ymd;

void StringStream::write(const char data){
    str_ += data;
}
void StringStream::write(const char * data_ptr, const size_t len) {
    str_.concat(data_ptr, len);
}
size_t StringStream::pending() const {
    return 0;
}

StringStream::operator String() const {
    return str_;
}
StringStream::operator StringView() const {
    return StringView(str_);
}