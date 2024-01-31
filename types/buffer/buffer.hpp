#ifndef __BUFFER_HPP__

#define __BUFFER_HPP__

#include "../../src/defines/comm_inc.h"
#include <cstdint>
#include <string>

template<typename T>
class Buffer_t{
protected:
    T * buf;
    size_t size;

    virtual T* advancePointer(T* ptr, size_t step) = 0;

public:
    Buffer_t(const size_t & _size = 128){
        size = _size;
        buf = new T[size];
    }

    ~Buffer_t(){
        delete buf;
    }

    virtual void addTxData(const T & data) = 0;
    virtual void getTxData(T & data) = 0;
    virtual void addRxData(const T & data) = 0;
    virtual void getRxData(T & data) = 0;
    virtual T * rxPtr() = 0;
    virtual T * txPtr() = 0;
    virtual size_t available() const = 0;
    virtual void addTxdatas(const T * data_ptr, const size_t & len, bool msb = false) = 0;
    virtual void getRxDatas(T * data_ptr, const size_t & len, bool msb = false) = 0;

};

typedef Buffer_t<uint8_t> Buffer;

#endif