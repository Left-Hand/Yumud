#pragma once

#include "../buffer.hpp"

template<typename T, size_t N>
class Fifo_t:public StaticBuffer_t<T, N>{
protected:
    // Pointer advance(Pointer ptr,const size_t step) {
    //     T * temp = const_cast<T *>(ptr) + step - N;

    //     if((temp >= this->buf)){
    //         // return const_cast<volatile T *>(temp);
    //         return temp;
    //     }else{
    //         // return const_cast<volatile T *>(temp + N);
    //         return temp + N;
    //     }
    // }
    // using Pointer = T * volatile;
    using Pointer = T *;

    __fast_inline bool over(Pointer ptr, const size_t step){
        return ptr + step >= this->buf + N;
    }

    Pointer advance(Pointer ptr, const size_t step) {
        // return over(ptr, step) ? ptr + step - N : ptr + step;
        return (ptr + step >= this->buf + N) ? ptr + step - N : ptr + step;
    }
public:
    Pointer read_ptr;
    Pointer write_ptr;

    Fifo_t():read_ptr(this->buf), write_ptr(this->buf){;}


    __fast_inline void push(const T & data) override{
        T * porg = (T *)write_ptr;
        write_ptr = advance(write_ptr, 1);
        *porg = data;
        if(write_ptr == read_ptr){
            read_ptr = advance(read_ptr, 1);
        }
    }

    __fast_inline void push(const T * pdata,const size_t len){
        // for(size_t i = 0; i < len; i++) push(pdata[i]);
        // //仅允许一次循环 如果有两次循环那便不是正确用法

        T * p_org = (T *)write_ptr;
        const int over = (write_ptr + len - N - this->buf);
        if(over >= 0){
            write_ptr = this->buf + over;

            const size_t len1 = N - (p_org - this->buf);
            const size_t len2 = over;

            memcpy(p_org, pdata, len1);
            memcpy(this->buf, pdata + len1, len2);
        }else{
            write_ptr = write_ptr + len;
            memcpy(p_org, pdata, len);
        }
    }

    void pop(T * pdata, const size_t len){
        for(size_t i = 0; i < len; i++) pdata[i] = pop();
        //仅允许一次循环 如果有两次循环那便不是正确用法

        // T * p_org = (T *)read_ptr;
        // const int over = (read_ptr + len - N - this->buf);
        // if(over >= 0){
        //     read_ptr = this->buf + over;

        //     const size_t len1 = N - (p_org - this->buf);
        //     const size_t len2 = over;

        //     for(size_t i = 0; i < len1; i++) pdata[i] = p_org[i];
        //     for(size_t i = 0; i < len2; i++) pdata[i + len1] = this->buf[i];
        //     // memcpy(pdata, p_org, len1);
        //     // memcpy(pdata + len1, this->buf, len2);
        // }else{
        //     read_ptr = pdata + len;
        //     // memcpy(pdata, p_org, len);
        //     for(size_t i = 0; i < len; i++) pdata[i] = p_org[i];
        // }
    }

    __fast_inline const T & pop() override{
        auto ret_ptr = read_ptr;
        read_ptr = advance(read_ptr, 1);
        return *(const T *)ret_ptr;
    }

    __fast_inline const T & front() {
        return *(const T *)read_ptr;
    }

    size_t available() const override{
        if (write_ptr >= read_ptr) {
            return size_t(write_ptr - read_ptr);
        } else {
            return N - size_t(read_ptr - write_ptr);
        }
    }

    volatile size_t straight() const{
        if (write_ptr >= read_ptr) {
            return write_ptr - read_ptr;
        }else{
            return (this->buf + this->size) - read_ptr;
        }
    }

    void vent(const size_t len){
        read_ptr = advance(read_ptr, len);
        return;
    }
};
template<uint32_t size>
using RingBuf = Fifo_t<uint8_t, size>;

