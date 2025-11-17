#pragma once

#include <cstdint>
#include <span>
#include <memory>
#include <utility>
#include <atomic>

#if 0
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
#endif

namespace ymd{

template<typename T, size_t N>
requires (std::has_single_bit(N))
class RingBuf final {
public:
    static constexpr size_t MASK = (N - 1);
private:
    alignas(T) uint8_t storage_[N * sizeof(T)];
    volatile size_t read_idx_ = 0;
    volatile size_t write_idx_ = 0;
    
    T* header_element_ptr() noexcept {
        return reinterpret_cast<T*>(storage_);
    }
    
    const T* header_element_ptr() const noexcept {
        return reinterpret_cast<const T*>(storage_);
    }
    
    static constexpr size_t advance(size_t idx, size_t step) noexcept {
        return (idx + step) & MASK;
    }
    
    [[nodiscard]] constexpr size_t read_idx() const {return read_idx_;}
    [[nodiscard]] constexpr size_t write_idx() const {return write_idx_;}

public:
    RingBuf() = default;
    
    ~RingBuf() {
        // 销毁所有已构造的对象
        while (read_idx_ != write_idx_) {
            std::destroy_at(header_element_ptr() + read_idx_);
            read_idx_ = advance(read_idx_, 1);
        }
    }
    
    // 禁止拷贝（右值安全）
    RingBuf(const RingBuf&) = delete;
    RingBuf& operator=(const RingBuf&) = delete;
    
    // 移动构造
    RingBuf(RingBuf&& other) noexcept {
        *this = std::move(other);
    }

    [[nodiscard]] static consteval size_t capacity() noexcept {
        return N;
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        std::construct_at(
            header_element_ptr() + write_idx_, 
            std::forward<Args>(args)...
        );
        write_idx_ = advance(write_idx_, 1);
    }

    [[nodiscard]] size_t push(std::span<const T> pdata) {
        size_t len = pdata.size();
        const size_t available = writable_size();
        
        len = std::min(len, available);
        
        if (write_idx_ + len < N) {
            // 单次拷贝
            for (size_t i = 0; i < len; i++) {
                std::construct_at(header_element_ptr() + write_idx_ + i, pdata[i]);
            }
        } else {
            // 环绕拷贝
            const size_t first_chunk = (N - 1) - write_idx_;
            const size_t second_chunk = len - first_chunk;
            
            // 第一段
            for (size_t i = 0; i < first_chunk; i++) {
                std::construct_at(header_element_ptr() + write_idx_ + i, pdata[i]);
            }
            
            // 第二段
            for (size_t i = 0; i < second_chunk; i++) {
                std::construct_at(header_element_ptr() + i, pdata[first_chunk + i]);
            }
        }
        
        write_idx_ = advance(write_idx_, len);
        return len;
    }

    [[nodiscard]] size_t pop(std::span<T> pdata) {
        const size_t len = std::min(pdata.size(), available());
        
        if (read_idx_ + len < N) {
            // 单次移动
            for (size_t i = 0; i < len; i++) {
                std::construct_at(
                    &pdata[i], 
                    header_element_ptr()[read_idx_ + i]
                );
                std::destroy_at(header_element_ptr() + read_idx_ + i);
            }
        } else {
            // 环绕移动
            const size_t first_chunk = (N - 1) - read_idx_;
            const size_t second_chunk = len - first_chunk;
            
            // 第一段
            for (size_t i = 0; i < first_chunk; i++) {
                std::construct_at(
                    &pdata[i], 
                    std::move(header_element_ptr()[read_idx_ + i])
                );
                std::destroy_at(header_element_ptr() + read_idx_ + i);
            }
            
            // 第二段
            for (size_t i = 0; i < second_chunk; i++) {
                std::construct_at(
                    &pdata[first_chunk + i], 
                    std::move(header_element_ptr()[i])
                );
                std::destroy_at(header_element_ptr() + i);
            }
        }
        
        read_idx_ = advance(read_idx_, len);
        return len;
    }

    [[nodiscard]] T pop() {
        T result = std::move(header_element_ptr()[read_idx_]);
        std::destroy_at(header_element_ptr() + read_idx_);
        read_idx_ = advance(read_idx_, 1);
        return result;  // 返回值，不是右值引用
    }


    void push(T&& data) {
        std::construct_at(
            header_element_ptr() + write_idx_, 
            std::move(data)
        );
        write_idx_ = advance(write_idx_, 1);
    }

    void push(const T& data) {
        std::construct_at(header_element_ptr() + write_idx_, data);
        write_idx_ = advance(write_idx_, 1u);
    }

    [[nodiscard]] __always_inline constexpr size_t available() const noexcept {
        const int diff = static_cast<int>(write_idx_) - static_cast<int>(read_idx_);
        if (diff >= 0) {
            return static_cast<size_t>(diff);
        } else {
            return static_cast<size_t>(static_cast<int>(N) + diff);
        }
    }

    [[nodiscard]] __always_inline constexpr size_t writable_size() const noexcept {
        return N - available();
    }


    __always_inline void waste(size_t len) noexcept {
        len = std::min(len, available());
        for (size_t i = 0; i < len; i++) {
            std::destroy_at(header_element_ptr() + read_idx_);
            read_idx_ = advance(read_idx_, 1);
        }
    }
    
};


}