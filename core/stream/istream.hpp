#pragma once

#include "stream_base.hpp"

namespace ymd{
class InputStream{
public:
    virtual ~InputStream() = default;

    virtual void read(char * data_ptr, const size_t len){
        for(size_t i=0;i<len;i++) read(data_ptr[i]);
    }

    virtual void read(char & data) = 0;
    
    char read(){char data; read(data); return data;};

    virtual size_t available() const = 0;
};

};