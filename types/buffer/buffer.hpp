#ifndef __BUFFER_HPP__

#define __BUFFER_HPP__

#include "src/comm_inc.h"
#include <cstdint>
#include <string>

template<typename T>
class Buffer_t{
protected:
    volatile T * buf;
    volatile size_t size;

public:
    Buffer_t(const size_t & _size = 128){
        size = (volatile size_t)_size;
        buf = new T[size];
    }

    ~Buffer_t(){
        delete buf;
    }

    virtual void addData(const T & data) = 0;
    virtual void getData(T & data) = 0;
    virtual size_t available() const = 0;
    virtual void addDatas(const T * data_ptr, const size_t & len, bool msb = false) = 0;
    virtual void getDatas(T * data_ptr, const size_t & len, bool msb = false) = 0;

};

typedef Buffer_t<uint8_t> Buffer;

#endif