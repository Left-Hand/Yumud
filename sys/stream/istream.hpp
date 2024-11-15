#pragma once

#include "stream_base.hpp"
#include "sys/string/string.hpp"

namespace ymd{
class InputStream:virtual public BasicStream{
public:
    virtual void read(char & data) = 0;
    virtual void read(char * data_ptr, const size_t len){
        for(size_t i=0;i<len;i++) read(data_ptr[i]);
    }

    char read(){char data; read(data); return data;};

    String readString(const size_t len);
    String readStringUntil(const char & chr);
    String readString(){return readString(available());}
    virtual size_t available() const = 0;
};

};