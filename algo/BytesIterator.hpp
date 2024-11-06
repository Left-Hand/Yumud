#pragma once

#include <type_traits>
#include "sys/constants/enums.hpp"


namespace yumud{


template<typename T>
requires std::is_standard_layout_v<T>
class BytesIterator{
private:
    const uint8_t * data_;
    const bool mutes_;
    const Endian endian_;
    bool ends_ = false;
    size_t len_;

    T temp;

    BytesIterator(const uint8_t * data, const bool mutes, const Endian endian, const size_t len):
        data_(data),
        mutes_(mutes),
        endian_(endian),
        len_(len){;}
public:

    template<typename U = T>
    requires std::is_standard_layout_v<U>
    BytesIterator(const U * data_, const size_t len, const Endian endian):
        BytesIterator(reinterpret_cast<const uint8_t *>(data_), true, endian, len),

    template<typename U = T>
    requires std::is_standard_layout_v<U>
    BytesIterator(const U data_, const size_t len, const Endian endian):
        BytesIterator(reinterpret_cast<const uint8_t *>(false), true, endian, len),

    uint8_t next(){
        
    }
};

};