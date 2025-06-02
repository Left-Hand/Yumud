#pragma once

#include "core/platform.hpp"
#include <span>

#ifndef likely
#define likely(x)      __builtin_expect(!!(x), 1)
#endif


#ifndef  unlikely
#define unlikely(x)    __builtin_expect(!!(x), 0)
#endif


namespace ymd{

template<typename T, size_t N>
requires (std::has_single_bit(N))
class Fifo_t{
public:
    Fifo_t() noexcept{;}

    __fast_inline constexpr size_t size() const noexcept{
        return N;
    }

    template <typename ... Args>
    void emplace(Args&&... args) noexcept {
        const auto w_org = windex;
        windex = forward_step(windex, 1);

        if constexpr (!std::is_trivially_destructible_v<T>) {
            (buf + w_org)->~T();  // Destroy existing object before reuse
        }
        new (buf + w_org) T(std::forward<Args>(args)...);
    }

    void push(std::span<const T> pbuf) noexcept {
        const auto w_org = windex;
        const size_t len = pbuf.size();
        const int32_t overlength = (windex + len - N);
        if(unlikely(overlength >= 0)){
            windex = overlength;

            const size_t len1 = N - w_org;
            const size_t len2 = overlength;

            for(size_t i = w_org; i < + w_org + len1; i++){
                if constexpr  (!std::is_trivially_destructible_v<T>)
                    (buf + i)->~T();  
                new (buf + i) T(pbuf[i]);
            }

            const T * last_data = &pbuf[len1];
            for(size_t i = 0; i < len2; i++){
                if constexpr  (!std::is_trivially_destructible_v<T>)
                    (buf + i)->~T(); 
                new (buf + i) T(last_data[i]);
            }
        }else{
            windex = windex + len;
            for(size_t i = w_org; i < w_org + len; i++){
                if constexpr  (!std::is_trivially_destructible_v<T>)
                    (buf + i)->~T(); 
                new (buf + i) T(pbuf[i]);
            }
        }
    }

    void pop(std::span<T> pbuf) noexcept {
        const auto w_org = (rindex);
        const uint32_t len = pbuf.size();
        const int32_t overlength = rindex + len - N;
        if(unlikely(overlength >= 0)){
            rindex = overlength;

            const uint32_t len1 = N - w_org;
            const uint32_t len2 = overlength;

            T * last_data = &pbuf[len1];

            for(uint32_t i = w_org; i < len1 + w_org; i++){
                pbuf[i] = std::move(buf[i]);
                if constexpr  (!std::is_trivially_destructible_v<T>)
                    (buf[i])->~T(); 
            }
            for(uint32_t i = 0; i < len2; i++){
                *(last_data + i) = std::move(buf[i]);
                if constexpr  (!std::is_trivially_destructible_v<T>)
                    (buf[i])->~T(); 
            }
        }else{
            rindex = rindex + len;
            for(uint32_t i = w_org; i < len + w_org; i++){
                pbuf[i] = std::move(buf[i]);
                if constexpr  (!std::is_trivially_destructible_v<T>)
                    (buf[i])->~T(); 
            }
        }

    }

    [[nodiscard]] __fast_inline const T pop() noexcept {
        const auto r_org = rindex;
        rindex = forward_step(rindex, 1);
        T tmp =  std::move(buf[r_org]);
        if constexpr (!std::is_trivially_destructible_v<T>) {
            (buf + r_org)->~T();
        }
        return tmp;
    }

    __fast_inline void push(const T & data) noexcept {
        const auto w_org = windex;
        windex = forward_step(windex, 1);
        new (buf + w_org) T(data);
    }

    __fast_inline void push(T && data) noexcept {
        const auto w_org = windex;
        windex = forward_step(windex, 1);
        if constexpr (!std::is_trivially_destructible_v<T>) {
            (buf + w_org)->~T();
        }
        new (buf + w_org) T(std::move(data));
    }

    [[nodiscard]] __fast_inline const T & front() noexcept {
        return *std::launder(buf + rindex);
    }

    [[nodiscard]] __fast_inline size_t available() const noexcept{
        if (windex >= rindex) {
            return windex - rindex;
        } else {
            return N - rindex + windex;
        }
    }

    [[nodiscard]] __fast_inline size_t safe_dma_length() const noexcept{
        //如果写入的索引在读出之后 那么可以直接截取读出后写入前的片段
        if (windex >= rindex) {
            return windex - rindex;
        }// 如果写入的索引在读出之前 那么只能截取读出前到数据末尾的片段
        else{
            return N - rindex;
        }
    }
    __fast_inline void waste(const size_t len) noexcept{
        rindex = forward_step(rindex, len);
    }

private:
    static_assert(std::is_nothrow_constructible_v<T>);
    
    T buf[N];

    size_t rindex = 0;
    size_t windex = 0;

    [[nodiscard]] size_t static __fast_inline constexpr 
    forward_step(const size_t index, const size_t step) noexcept{
        static constexpr uint32_t MASK = uint32_t(N - 1);
        return (index + step) & MASK;
    }
};
template<uint32_t size>
using RingBuf = Fifo_t<uint8_t, size>;

}