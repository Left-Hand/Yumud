#pragma once

#include "sys/stream/ostream.hpp"
#include "sys/string/string.hpp"


namespace ymd{

class StringStream:public OutputStream{
protected:
    String str_;
public:
    StringStream(){;}

    void write(const char data) override;
    void write(const char * data_ptr, const size_t len) override;
    size_t pending() const override;

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