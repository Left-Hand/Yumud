#pragma once

#include "../sys/core/platform.h"
#include <cstdint>
#include <string>
#include <array>

template<typename T>
class BufferConcept_t{
public:
    virtual void push(const T & data) = 0;
    virtual const T & pop() = 0;
    virtual size_t available() const = 0;
    virtual void push(const T * data_ptr, const size_t len, bool msb = false) = 0;
    virtual void pop(T * data_ptr, const size_t len, bool msb = false) = 0;
};


template<typename T>
class DynamicBuffer_t:public BufferConcept_t<T>{
protected:
    T * buf;
    size_t size;

public:
    DynamicBuffer_t(const size_t _size = 128){
        size = (volatile size_t)_size;
        buf = new T[size];
    }

    ~DynamicBuffer_t(){
        delete buf;
    }
};

template<typename T, uint32_t _size>
class StaticBuffer_t:public BufferConcept_t<T>{
public:
    T buf[_size]; 
    size_t size = _size;
    StaticBuffer_t() = default;
    ~StaticBuffer_t() = default;

    T & operator[](const size_t index){
        return buf[index];
    }

    constexpr size_t getSize() const {
        return size;
    }
};

typedef DynamicBuffer_t<uint8_t> DynamicBuffer;

template<uint32_t size>
using StaticBuffer = StaticBuffer_t<uint8_t,size>;
