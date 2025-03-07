#pragma once

#include "sys/core/platform.h"

template<typename T, size_t N>
class Fifo_t{
protected:
    T buf[N];
    using Pointer = T *;

    __fast_inline bool over(Pointer ptr, const size_t step){
        return ptr + step >= this->buf + N;
    }

    T * advance(T * ptr, const size_t step) {
        return (ptr + step >= this->buf + N) ? ptr + step - N : ptr + step;
    }

    const T * advance(const T * ptr, const size_t step) const {
        return (ptr + step >= this->buf + N) ? ptr + step - N : ptr + step;
    }
public:
    Pointer read_ptr;
    Pointer write_ptr;

    Fifo_t():read_ptr(this->buf), write_ptr(this->buf){;}

    __fast_inline constexpr size_t size() const {
        return N;
    }

    __fast_inline void push(auto && data){
        T * porg = write_ptr;
        write_ptr = advance(write_ptr, 1);
        new (porg) T(data);
        // if(write_ptr == read_ptr){
        //     read_ptr = advance(read_ptr, 1);
        // }
    }

    template <typename ... Args>
    __fast_inline void emplace(Args&&... args){
        T * porg = write_ptr;
        write_ptr = advance(write_ptr, 1);
        new (porg) T(std::forward<Args>(args)...);
        // if (write_ptr == read_ptr) {
        //     read_ptr = advance(read_ptr, 1);
        // }
    }

    __fast_inline void push(const T * pdata,const size_t len){
        T * p_org = write_ptr;
        const int over = (write_ptr + len - N - this->buf);
        if(over >= 0){
            write_ptr = this->buf + over;

            const size_t len1 = N - (p_org - this->buf);
            const size_t len2 = over;

            for(size_t i = 0; i < len1; i++){
                new (p_org + i) T(pdata[i]);
            }

            const T * last_data = pdata + len1;
            for(size_t i = 0; i < len2; i++){
                new (this->buf + i) T(last_data[i]);
            }
        }else{
            write_ptr = write_ptr + len;
            for(size_t i = 0; i < len; i++){
                new (p_org + i) T(pdata[i]);
            }
        }
    }

    void pop(T * pdata, const size_t len){
        T * p_org = (read_ptr);
        const int over = (read_ptr + len - N - this->buf);
        if(over >= 0){
            read_ptr = this->buf + over;

            const size_t len1 = N - (p_org - this->buf);
            const size_t len2 = over;

            T * last_data = pdata + len1;

            for(size_t i = 0; i < len1; i++) new (pdata + i) T(p_org[i]);
            for(size_t i = 0; i < len2; i++) new (last_data + i) T(this->buf[i]);
        }else{
            read_ptr = read_ptr + len;
            for(size_t i = 0; i < len; i++) new (pdata + i) T(p_org[i]);
        }
    }

    __fast_inline const T && pop(){
        const T * ret_ptr = read_ptr;
        read_ptr = advance(read_ptr, 1);
        return std::move(*ret_ptr);
    }

    __fast_inline const T & front() {
        return *read_ptr;
    }

    size_t available() const {
        if (write_ptr >= read_ptr) {
            return size_t(write_ptr - read_ptr);
        } else {
            return N - size_t(read_ptr - write_ptr);
        }
    }

    size_t straight() const{
        if (write_ptr >= read_ptr) {
            return write_ptr - read_ptr;
        }else{
            return (this->buf + this->size) - read_ptr;
        }
    }

    __fast_inline const T foresee(const size_t idx) const{
        return *advance(read_ptr, idx);
    }
    void vent(const size_t len){
        read_ptr = advance(read_ptr, len);
        return;
    }
};
template<uint32_t size>
using RingBuf = Fifo_t<uint8_t, size>;

