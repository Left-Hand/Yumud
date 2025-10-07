#pragma once

#include <cstdint>
#include <span>
#include <memory>
#include <utility>


#ifndef likely
#define likely(x)      __builtin_expect(!!(x), 1)
#endif


#ifndef  unlikely
#define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

namespace ymd{

#if 0

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
#else

template<typename T, size_t N>
requires (std::has_single_bit(N))
class RingBuf final {
public:
    static constexpr size_t MASK = (N - 1);
private:
    alignas(T) uint8_t storage[N * sizeof(T)];
    volatile size_t read_idx = 0;
    volatile size_t write_idx = 0;
    
    T* data_ptr() noexcept {
        return reinterpret_cast<T*>(storage);
    }
    
    const T* data_ptr() const noexcept {
        return reinterpret_cast<const T*>(storage);
    }
    
    static constexpr size_t advance(size_t idx, size_t step) noexcept {
        return (idx + step) & MASK;
    }
    
    // 循环展开辅助函数
    template<size_t Start, size_t End, size_t Step = 1>
    struct Unroller {
        template<typename Func>
        static void apply(Func&& func) {
            func(Start);
            Unroller<Start + Step, End, Step>::apply(std::forward<Func>(func));
        }
    };
    
    template<size_t End, size_t Step>
    struct Unroller<End, End, Step> {
        template<typename Func>
        static void apply(Func&&) {}
    };

public:
    RingBuf() = default;
    
    ~RingBuf() {
        // 销毁所有已构造的对象
        while (read_idx != write_idx) {
            std::destroy_at(data_ptr() + read_idx);
            read_idx = advance(read_idx, 1);
        }
    }
    
    // 禁止拷贝（右值安全）
    RingBuf(const RingBuf&) = delete;
    RingBuf& operator=(const RingBuf&) = delete;
    
    // 移动构造
    RingBuf(RingBuf&& other) noexcept {
        *this = std::move(other);
    }
    
    #if 0
    // 移动赋值
    RingBuf& operator=(RingBuf&& other) noexcept {
        if (this != &other) {
            // 销毁当前对象
            while (read_idx != write_idx) {
                std::destroy_at(data_ptr() + read_idx);
                read_idx = advance(read_idx, 1);
            }
            
            // 移动数据
            while (other.read_idx != other.write_idx) {
                std::construct_at(
                    data_ptr() + write_idx, 
                    std::move(other.data_ptr()[other.read_idx])
                );
                std::destroy_at(other.data_ptr() + other.read_idx);
                write_idx = advance(write_idx, 1);
                other.read_idx = other.advance(other.read_idx, 1);
            }
        }
        return *this;
    }
    #endif

    inline constexpr size_t size() const noexcept {
        return N;
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        std::construct_at(
            data_ptr() + write_idx, 
            std::forward<Args>(args)...
        );
        write_idx = advance(write_idx, 1);
    }

    [[nodiscard]] size_t push(std::span<const T> pdata) {
        size_t len = pdata.size();
        const size_t available = writable_capacity();
        
        len = std::min(len, available);
        
        if (write_idx + len < N) {
            // 单次拷贝
            for (size_t i = 0; i < len; i++) {
                std::construct_at(data_ptr() + write_idx + i, pdata[i]);
            }
        } else {
            // 环绕拷贝
            const size_t first_chunk = (N - 1) - write_idx;
            const size_t second_chunk = len - first_chunk;
            
            // 第一段
            for (size_t i = 0; i < first_chunk; i++) {
                std::construct_at(data_ptr() + write_idx + i, pdata[i]);
            }
            
            // 第二段
            for (size_t i = 0; i < second_chunk; i++) {
                std::construct_at(data_ptr() + i, pdata[first_chunk + i]);
            }
        }
        
        write_idx = advance(write_idx, len);
        return len;
    }

    [[nodiscard]] size_t pop(std::span<T> pdata) {
        const size_t len = std::min(pdata.size(), available());
        
        if (read_idx + len < N) {
            // 单次移动
            for (size_t i = 0; i < len; i++) {
                std::construct_at(
                    &pdata[i], 
                    data_ptr()[read_idx + i]
                );
                std::destroy_at(data_ptr() + read_idx + i);
            }
        } else {
            // 环绕移动
            const size_t first_chunk = (N - 1) - read_idx;
            const size_t second_chunk = len - first_chunk;
            
            // 第一段
            for (size_t i = 0; i < first_chunk; i++) {
                std::construct_at(
                    &pdata[i], 
                    std::move(data_ptr()[read_idx + i])
                );
                std::destroy_at(data_ptr() + read_idx + i);
            }
            
            // 第二段
            for (size_t i = 0; i < second_chunk; i++) {
                std::construct_at(
                    &pdata[first_chunk + i], 
                    std::move(data_ptr()[i])
                );
                std::destroy_at(data_ptr() + i);
            }
        }
        
        read_idx = advance(read_idx, len);
        return len;
    }

    [[nodiscard]] T pop() {
        T result = std::move(data_ptr()[read_idx]);
        std::destroy_at(data_ptr() + read_idx);
        read_idx = advance(read_idx, 1);
        return result;  // 返回值，不是右值引用
    }

    void push(T&& data) {
        std::construct_at(
            data_ptr() + write_idx, 
            std::move(data)
        );
        write_idx = advance(write_idx, 1);
    }

    void push(const T& data) {
        std::construct_at(data_ptr() + write_idx, data);
        write_idx = advance(write_idx, 1);
    }

    [[nodiscard]] inline constexpr size_t available() const noexcept {
        if (write_idx >= read_idx) {
            return write_idx - read_idx;
        } else {
            return N - (read_idx - write_idx);
        }
    }

    [[nodiscard]] inline constexpr size_t writable_capacity() const noexcept {
        return N - available();
    }

    [[nodiscard]] inline constexpr size_t safe_dma_length() const noexcept {
        if (write_idx >= read_idx) {
            return write_idx - read_idx;
        } else {
            return N - read_idx;
        }
    }

    inline void waste(size_t len) noexcept {
        len = std::min(len, available());
        for (size_t i = 0; i < len; i++) {
            std::destroy_at(data_ptr() + read_idx);
            read_idx = advance(read_idx, 1);
        }
    }
    
    // 循环展开版本的批量操作（可选优化）
    template<size_t BatchSize>
    void push_batch(const T (&data)[BatchSize]) {
        static_assert(BatchSize <= N, "Batch size exceeds buffer size");
        
        Unroller<0, BatchSize>::apply([&](size_t i) {
            std::construct_at(data_ptr() + write_idx, data[i]);
            write_idx = advance(write_idx, 1);
        });
    }
};
#endif

}