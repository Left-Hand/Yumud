#pragma once

#include "core/stream/ostream.hpp"
#include "core/string/string.hpp"


namespace ymd{

class StringStream:public OutputStream{
protected:
    String str_;
public:
    StringStream(){;}

    size_t pending() const;

    String && move_str() && {
        flush();
        return std::move(str_);
    }

    void reserve(const size_t len){
        str_.reserve(len);
    }

    String copy() const{
        return String(str_);
    }

    void sendout(const std::span<const char> pbuf){
        str_.concat(pbuf.data(), pbuf.size());
    }

    explicit operator String() const;
    explicit operator StringView() const;
};

};