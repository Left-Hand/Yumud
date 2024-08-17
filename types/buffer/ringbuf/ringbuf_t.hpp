#ifndef __RING_BUF_HPP

#define __RING_BUF_HPP

#include "../buffer.hpp"

template<typename T, uint32_t _size>
class RingBuf_t:public StaticBuffer_t<T, _size>{
protected:
    volatile T * advancePointer(volatile T * ptr, size_t step = 1) {
        return (ptr + step >=this->buf + this->size) ? ptr + step - this->size : ptr + step;
    }

public:
    volatile T * read_ptr;
    volatile T * write_ptr;

    RingBuf_t():read_ptr(this->buf), write_ptr(this->buf){;}


    __fast_inline void addData(const T & data) override{
        *(T *)write_ptr = data;
        write_ptr = advancePointer(write_ptr);
        if(write_ptr == read_ptr){
            read_ptr = advancePointer(read_ptr);
        }
    }

    __fast_inline void addData(const T * data,const size_t data_size){
        for(size_t i = 0; i < data_size; i++){
            addData(data[data_size]);
        }
    }
    //     auto ptr_before = write_ptr;
    //     // *write_ptr = data;
    //     memcpy(write_ptr, data, )
    //     write_ptr = advancePointer(write_ptr);
    //     if(write_ptr == read_ptr){
    //         read_ptr = advancePointer(read_ptr);
    //     }
    // }

    __fast_inline const T & getData() override{
        auto ret_ptr = read_ptr;
        read_ptr = advancePointer(read_ptr);
        return *(const T *)ret_ptr;
    }

    __fast_inline const T & front() {
        return *(const T *)read_ptr;
    }

    size_t available() const override{
        if (write_ptr >= read_ptr) {
            return write_ptr - read_ptr;
        } else {
            return this->size - (read_ptr - write_ptr);
        }
    }

    volatile size_t straight() const{
        if (write_ptr >= read_ptr) {
            return write_ptr - read_ptr;
        }else{
            return (this->buf + this->size) - read_ptr;
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

    void vent(const size_t len){
        read_ptr = advancePointer(read_ptr, len);
        return;
    }
};
template<uint32_t size>
using RingBuf = RingBuf_t<uint8_t, size>;

#endif // !__RING_BUF_HPP