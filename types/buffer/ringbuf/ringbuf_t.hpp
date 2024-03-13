#ifndef __RING_BUF_HPP

#define __RING_BUF_HPP

#include "../buffer.hpp"

template<typename T, uint32_t _size>
class RingBuf_t:public StaticBuffer_t<T, _size>{
protected:
    T * write_ptr;
    T * read_ptr;

    T* advancePointer(T* ptr, size_t step = 1) {
        return (ptr + step >=this->buf + this->size) ? ptr + step - this->size : ptr + step;
    }

public:
    RingBuf_t():write_ptr(this->buf), read_ptr(this->buf){;}


    __fast_inline void addData(const T & data) override{
        *read_ptr = data;
        read_ptr = advancePointer(read_ptr);
        if(read_ptr == write_ptr){
            write_ptr = advancePointer(write_ptr);
        }
    }

    // __fast_inline void getData(T & data) override{
    //     data = *write_ptr;
    //     write_ptr = advancePointer(write_ptr);
    // }

    __fast_inline T & getData() override{
        auto ret_ptr = write_ptr;
        write_ptr = advancePointer(write_ptr);
        return *ret_ptr;
    }

    size_t available() const override{
        if (read_ptr >= write_ptr) {
            return read_ptr - write_ptr;
        } else {
            return this->size - (write_ptr - read_ptr);
        }
    }

    void addDatas(const T * data_ptr, const size_t & len, bool msb = false) override{
        if(msb){
            for(size_t i = len - 1; i > 0; i--) addData(data_ptr[i]);
        }else{
            for(size_t i = 0; i < len; i++) addData(data_ptr[i]);
        }
    }

    void getDatas(T * data_ptr, const size_t & len, bool msb = false) override{
        if(msb){
            for(size_t i = len - 1; i > 0; i--) data_ptr[i] = getData();
        }else{
            for(size_t i = 0; i < len; i++) data_ptr[i] = getData();
        }
    }
};
template<uint32_t size>
using RingBuf = RingBuf_t<uint8_t, size>;

#endif // !__RING_BUF_HPP