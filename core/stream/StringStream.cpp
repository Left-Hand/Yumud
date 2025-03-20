#include "StringStream.hpp"


using namespace ymd;

void StringStream::write(const char data){
    str_.concat(&data, 1);
}

void StringStream::write(const char * data_ptr, const size_t len) {
    str_.concat(data_ptr, len);
    // const auto org_len = str_.length();
    // const auto exp_len = org_len + len + 1;

    // if(exp_len > str_.capacity()){
    //     str_.reserve(exp_len);
    // }

    // auto ptr = &(str_[org_len]);
    // for(size_t i = 0; i < len; i++){
    //     ptr[i] = data_ptr[i];
    // }
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