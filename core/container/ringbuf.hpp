#pragma once

#include <cstdint>
#include <span>

#ifndef likely
#define likely(x)      __builtin_expect(!!(x), 1)
#endif


#ifndef  unlikely
#define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

namespace ymd{

template<typename T, size_t N>
class RingBuf final{
private:
    T buf[N];
    using Pointer = T *;

    inline bool over(Pointer ptr, const size_t step){
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

    RingBuf():read_ptr(this->buf), write_ptr(this->buf){;}

    inline constexpr size_t size() const {
        return N;
    }

    template <typename ... Args>
    void emplace(Args&&... args){
        T * porg = write_ptr;
        write_ptr = advance(write_ptr, 1);
        new (porg) T(std::forward<Args>(args)...);
    }

    [[nodiscard]] size_t push(std::span<const T> pdata){
        T * p_org = write_ptr;
        const size_t len = pdata.size();
        const int over = (write_ptr + len - N - this->buf);
        if(over >= 0){
            write_ptr = this->buf + over;

            const size_t len1 = N - (p_org - this->buf);
            const size_t len2 = over;

            for(size_t i = 0; i < len1; i++){
                new (p_org + i) T(pdata[i]);
            }

            const T * last_data = &pdata[len1];
            for(size_t i = 0; i < len2; i++){
                new (this->buf + i) T(last_data[i]);
            }
        }else{
            write_ptr = write_ptr + len;
            for(size_t i = 0; i < len; i++){
                new (p_org + i) T(pdata[i]);
            }
        }

        return len;
    }

    [[nodiscard]] size_t pop(std::span<T> pdata){
        T * p_org = (read_ptr);
        const size_t len = pdata.size();
        const int over = (read_ptr + len - N - this->buf);
        if(over >= 0){
            read_ptr = this->buf + over;

            const size_t len1 = N - (p_org - this->buf);
            const size_t len2 = over;

            T * last_data = &pdata[len1];

            for(size_t i = 0; i < len1; i++) new (&pdata[i]) T(p_org[i]);
            for(size_t i = 0; i < len2; i++) new (last_data + i) T(this->buf[i]);
        }else{
            read_ptr = read_ptr + len;
            for(size_t i = 0; i < len; i++) new (&pdata[i]) T(p_org[i]);
        }

        return len;
    }

    [[nodiscard]] inline const T && pop(){
        const T * ret_ptr = read_ptr;
        read_ptr = advance(read_ptr, 1);
        return std::move(*ret_ptr);
    }

    inline void push(const T & data){
        T * porg = write_ptr;
        write_ptr = advance(write_ptr, 1);
        new (porg) T(data);
    }

    [[nodiscard]] inline const T & front() {
        return *read_ptr;
    }

    [[nodiscard]] inline size_t available() const {
        if (write_ptr >= read_ptr) {
            return size_t(write_ptr - read_ptr);
        } else {
            return N - size_t(read_ptr - write_ptr);
        }
    }

    [[nodiscard]] inline size_t writable_capacity() const {
        return N - available();
    }

    [[nodiscard]] inline size_t safe_dma_length() const{
        if (write_ptr >= read_ptr) {
            return write_ptr - read_ptr;
        }else{
            return (this->buf + this->size) - read_ptr;
        }
    }


    inline void waste(const size_t len){
        read_ptr = advance(read_ptr, len);
        return;
    }
};


}