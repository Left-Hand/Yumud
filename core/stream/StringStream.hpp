#pragma once

#include "core/stream/ostream.hpp"
#include "core/string/string.hpp"


namespace ymd{

class StringStream:public OutputStream{
protected:
    String str_;
public:
    StringStream(){;}

    void write(const char data);
    void write(const char * data_ptr, const size_t len);
    size_t pending() const;

    String && move_str() && {
        return std::move(str_);
    }

    void reserve(const size_t len){
        str_.reserve(len);
    }

    String copy() const{
        return String(str_);
    }

    explicit operator String() const;
    explicit operator StringView() const;
};

};