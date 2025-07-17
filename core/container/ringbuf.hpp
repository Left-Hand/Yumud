// #pragma once

// #include "core/platform.hpp"
// #include <span>

// #ifndef likely
// #define likely(x)      __builtin_expect(!!(x), 1)
// #endif


// #ifndef  unlikely
// #define unlikely(x)    __builtin_expect(!!(x), 0)
// #endif


// namespace ymd{

// template<typename T, size_t N>
// requires (std::has_single_bit(N))
// class RingBuf{
// public:
//     RingBuf() noexcept{;}

//     __fast_inline constexpr size_t size() const noexcept{
//         return N;
//     }

//     template <typename ... Args>
//     void emplace(Args&&... args) noexcept {
//         const auto w_org = windex;
//         windex = forward_step(windex, 1);

//         if constexpr (!std::is_trivially_destructible_v<T>) {
//             (buf + w_org)->~T();  // Destroy existing object before reuse
//         }
//         new (buf + w_org) T(std::forward<Args>(args)...);
//     }

//     void push(std::span<const T> pbuf) noexcept {
//         const auto w_org = windex;
//         const size_t len = pbuf.size();
//         const int32_t overlength = (windex + len - N);
//         if(unlikely(overlength >= 0)){
//             windex = overlength;

//             const size_t len1 = N - w_org;
//             const size_t len2 = overlength;

//             for(size_t i = w_org; i < + w_org + len1; i++){
//                 if constexpr  (!std::is_trivially_destructible_v<T>)
//                     (buf + i)->~T();  
//                 new (buf + i) T(pbuf[i]);
//             }

//             const T * last_data = &pbuf[len1];
//             for(size_t i = 0; i < len2; i++){
//                 if constexpr  (!std::is_trivially_destructible_v<T>)
//                     (buf + i)->~T(); 
//                 new (buf + i) T(last_data[i]);
//             }
//         }else{
//             windex = windex + len;
//             for(size_t i = w_org; i < w_org + len; i++){
//                 if constexpr  (!std::is_trivially_destructible_v<T>)
//                     (buf + i)->~T(); 
//                 new (buf + i) T(pbuf[i]);
//             }
//         }
//     }

//     void pop(std::span<T> pbuf) noexcept {
//         const auto w_org = (rindex);
//         const uint32_t len = pbuf.size();
//         const int32_t overlength = rindex + len - N;
//         if(unlikely(overlength >= 0)){
//             rindex = overlength;

//             const uint32_t len1 = N - w_org;
//             const uint32_t len2 = overlength;

//             T * last_data = &pbuf[len1];

//             for(uint32_t i = w_org; i < len1 + w_org; i++){
//                 pbuf[i] = std::move(buf[i]);
//                 if constexpr  (!std::is_trivially_destructible_v<T>)
//                     (buf[i])->~T(); 
//             }
//             for(uint32_t i = 0; i < len2; i++){
//                 *(last_data + i) = std::move(buf[i]);
//                 if constexpr  (!std::is_trivially_destructible_v<T>)
//                     (buf[i])->~T(); 
//             }
//         }else{
//             rindex = rindex + len;
//             for(uint32_t i = w_org; i < len + w_org; i++){
//                 pbuf[i] = std::move(buf[i]);
//                 if constexpr  (!std::is_trivially_destructible_v<T>)
//                     (buf[i])->~T(); 
//             }
//         }

//     }

//     [[nodiscard]] __fast_inline const T pop() noexcept {
//         const auto r_org = rindex;
//         rindex = forward_step(rindex, 1);
//         T tmp =  std::move(buf[r_org]);
//         if constexpr (!std::is_trivially_destructible_v<T>) {
//             (buf + r_org)->~T();
//         }
//         return tmp;
//     }

//     __fast_inline void push(const T & data) noexcept {
//         const auto w_org = windex;
//         windex = forward_step(windex, 1);
//         new (buf + w_org) T(data);
//     }

//     __fast_inline void push(T && data) noexcept {
//         const auto w_org = windex;
//         windex = forward_step(windex, 1);
//         if constexpr (!std::is_trivially_destructible_v<T>) {
//             (buf + w_org)->~T();
//         }
//         new (buf + w_org) T(std::move(data));
//     }

//     [[nodiscard]] __fast_inline const T & front() noexcept {
//         return *std::launder(buf + rindex);
//     }

//     [[nodiscard]] __fast_inline size_t available() const noexcept{
//         if (windex >= rindex) {
//             return windex - rindex;
//         } else {
//             return N - rindex + windex;
//         }
//     }

//     [[nodiscard]] __fast_inline size_t safe_dma_length() const noexcept{
//         //如果写入的索引在读出之后 那么可以直接截取读出后写入前的片段
//         if (windex >= rindex) {
//             return windex - rindex;
//         }// 如果写入的索引在读出之前 那么只能截取读出前到数据末尾的片段
//         else{
//             return N - rindex;
//         }
//     }
//     __fast_inline void waste(const size_t len) noexcept{
//         rindex = forward_step(rindex, len);
//     }

// private:
//     static_assert(std::is_nothrow_constructible_v<T>);
    
//     T buf[N];

//     size_t rindex = 0;
//     size_t windex = 0;

//     [[nodiscard]] size_t static __fast_inline constexpr 
//     forward_step(const size_t index, const size_t step) noexcept{
//         static constexpr uint32_t MASK = uint32_t(N - 1);
//         return (index + step) & MASK;
//     }
// };
// template<uint32_t size>
// using RingBuf = RingBuf<uint8_t, size>;

// }

#pragma once

#include "core/platform.hpp"
#include <span>

namespace ymd{

template<typename T, size_t N>
class RingBuf{
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

    RingBuf():read_ptr(this->buf), write_ptr(this->buf){;}

    __fast_inline constexpr size_t size() const {
        return N;
    }

    template <typename ... Args>
    void emplace(Args&&... args){
        T * porg = write_ptr;
        write_ptr = advance(write_ptr, 1);
        new (porg) T(std::forward<Args>(args)...);
    }

    size_t push(std::span<const T> pdata){
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

    size_t pop(std::span<T> pdata){
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

    __fast_inline const T && pop(){
        const T * ret_ptr = read_ptr;
        read_ptr = advance(read_ptr, 1);
        return std::move(*ret_ptr);
    }

    __fast_inline void push(const T & data){
        T * porg = write_ptr;
        write_ptr = advance(write_ptr, 1);
        new (porg) T(data);
    }

    __fast_inline const T & front() {
        return *read_ptr;
    }

    __fast_inline size_t available() const {
        if (write_ptr >= read_ptr) {
            return size_t(write_ptr - read_ptr);
        } else {
            return N - size_t(read_ptr - write_ptr);
        }
    }

    __fast_inline size_t straight() const{
        if (write_ptr >= read_ptr) {
            return write_ptr - read_ptr;
        }else{
            return (this->buf + this->size) - read_ptr;
        }
    }

    __fast_inline const T foresee(const size_t idx) const{
        return *advance(read_ptr, idx);
    }
    __fast_inline void vent(const size_t len){
        read_ptr = advance(read_ptr, len);
        return;
    }
};


}