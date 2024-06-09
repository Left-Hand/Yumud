#ifndef __RING_BUF_HPP

#define __RING_BUF_HPP

#include "../buffer.hpp"

template<typename T, uint32_t _size>
class RingBuf_t:public StaticBuffer_t<T, _size>{
protected:
    T* advancePointer(T* ptr, size_t step = 1) {
        return (ptr + step >=this->buf + this->size) ? ptr + step - this->size : ptr + step;
    }

public:
    T * rd_ptr;
    T * wr_ptr;

    RingBuf_t():rd_ptr(this->buf), wr_ptr(this->buf){;}


    __fast_inline void addData(const T & data) override{
        *wr_ptr = data;
        wr_ptr = advancePointer(wr_ptr);
        if(wr_ptr == rd_ptr){
            rd_ptr = advancePointer(rd_ptr);
        }
    }

    __fast_inline T & getData() override{
        auto ret_ptr = rd_ptr;
        rd_ptr = advancePointer(rd_ptr);
        return *ret_ptr;
    }

    size_t available() const override{
        if (wr_ptr >= rd_ptr) {
            return wr_ptr - rd_ptr;
        } else {
            return this->size - (rd_ptr - wr_ptr);
        }
    }

    size_t straight() const{
        if (wr_ptr >= rd_ptr) {
            return wr_ptr - rd_ptr;
        }else{
            return this->size - (rd_ptr - this->buf);
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

    void readForward(const size_t len){
        rd_ptr = advancePointer(rd_ptr, len);
        return;
    }
};
template<uint32_t size>
using RingBuf = RingBuf_t<uint8_t, size>;

#endif // !__RING_BUF_HPP