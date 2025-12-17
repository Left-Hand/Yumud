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

// template<typename T, size_t N>
// requires (std::has_single_bit(N))
// class RingBuf final {
// public:
//     static constexpr size_t MASK = (N - 1);
// private:
//     alignas(T) uint8_t storage_[N * sizeof(T)];
//     volatile size_t read_idx_ = 0;
//     volatile size_t write_idx_ = 0;
    
//     T* header_ptr() noexcept {
//         return reinterpret_cast<T*>(storage_);
//     }
    
//     const T* header_ptr() const noexcept {
//         return reinterpret_cast<const T*>(storage_);
//     }
    
//     static constexpr size_t advance(size_t idx, size_t step) noexcept {
//         return (idx + step) & MASK;
//     }
    
//     [[nodiscard]] constexpr size_t read_idx() const {return read_idx_;}
//     [[nodiscard]] constexpr size_t write_idx() const {return write_idx_;}

// public:
//     RingBuf() = default;
    
//     ~RingBuf() {
//         waste(length());
//     }
    
//     // 禁止拷贝（右值安全）
//     RingBuf(const RingBuf&) = delete;
//     RingBuf& operator=(const RingBuf&) = delete;
    
//     // 移动构造
//     RingBuf(RingBuf&& other) noexcept {
//         *this = std::move(other);
//     }

//     [[nodiscard]] static consteval size_t capacity() noexcept {
//         return N;
//     }

//     template<typename... Args>
//     void emplace(Args&&... args) {
//         std::construct_at(
//             header_ptr() + write_idx_, 
//             std::forward<Args>(args)...
//         );
//         write_idx_ = advance(write_idx_, 1);
//     }

//     [[nodiscard]] size_t try_push(std::span<const T> pdata) {
//         const auto len = std::min(pdata.size(), free_capacity());
//         const auto write_idx = write_idx_;
//         write_idx_ = advance(write_idx_, len);

//         if (write_idx + len < N) {
//             // 单次拷贝
//             for (size_t i = 0; i < len; i++) {
//                 std::construct_at(header_ptr() + write_idx + i, pdata[i]);
//             }
//         } else {
//             // 环绕拷贝
//             const size_t first_chunk = (N - 1) - write_idx;
//             const size_t second_chunk = len - first_chunk;
            
//             // 第一段
//             for (size_t i = 0; i < first_chunk; i++) {
//                 std::construct_at(header_ptr() + write_idx + i, pdata[i]);
//             }
            
//             // 第二段
//             for (size_t i = 0; i < second_chunk; i++) {
//                 std::construct_at(header_ptr() + i, pdata[first_chunk + i]);
//             }
//         }

//         return len;
//     }

//     [[nodiscard]] size_t try_pop(std::span<T> pdata) {
//         const size_t len = std::min(pdata.size(), length());
//         const auto read_idx = read_idx_; 
//         read_idx_ = advance(read_idx_, len);
//         if (read_idx + len < N) {
//             // 单次移动
//             for (size_t i = 0; i < len; i++) {
//                 std::construct_at(
//                     &pdata[i], 
//                     header_ptr()[read_idx + i]
//                 );
//                 std::destroy_at(header_ptr() + read_idx + i);
//             }
//         } else {
//             // 环绕移动
//             const size_t first_chunk = (N - 1) - read_idx;
//             const size_t second_chunk = len - first_chunk;
            
//             // 第一段
//             for (size_t i = 0; i < first_chunk; i++) {
//                 std::construct_at(
//                     &pdata[i], 
//                     std::move(header_ptr()[read_idx + i])
//                 );
//                 std::destroy_at(header_ptr() + read_idx + i);
//             }
            
//             // 第二段
//             for (size_t i = 0; i < second_chunk; i++) {
//                 std::construct_at(
//                     &pdata[first_chunk + i], 
//                     std::move(header_ptr()[i])
//                 );
//                 std::destroy_at(header_ptr() + i);
//             }
//         }
        

//         return len;
//     }

//     [[nodiscard]] T pop() {
//         T result = header_ptr()[read_idx_];
//         const auto read_idx = read_idx_; 
//         read_idx_ = advance(read_idx_, 1);
//         std::destroy_at(header_ptr() + read_idx);
//         return result;  // 返回值，不是右值引用
//     }

//     void push(const T& element) {
//         std::construct_at(
//             header_ptr() + write_idx_, 
//             element
//         );
//         write_idx_ = advance(write_idx_, 1);
//     }


//     [[nodiscard]] __always_inline constexpr size_t length() const noexcept {
//         const int diff = static_cast<int>(write_idx_) - static_cast<int>(read_idx_);
//         if (diff >= 0) {
//             return static_cast<size_t>(diff);
//         } else {
//             return static_cast<size_t>(static_cast<int>(N) + diff);
//         }
//     }

//     [[nodiscard]] __always_inline constexpr bool is_empty() const noexcept{
//         return length() == 0;
//     }

//     [[nodiscard]] __always_inline constexpr size_t free_capacity() const noexcept {
//         return N - length();
//     }


//     __always_inline void waste(size_t len) noexcept {
//         len = std::min(len, length());
//         for (size_t i = 0; i < len; i++) {
//             std::destroy_at(header_ptr() + read_idx_);
//             read_idx_ = advance(read_idx_, 1);
//         }
//     }
    
// };



template<typename T, size_t N>
requires (std::has_single_bit(N))
class RingBuf final {
public:
    static constexpr size_t MASK = N - 1;
    static constexpr size_t MAX_CAPACITY = N - 1;  // 保留一个空位区分空/满
    
private:
    alignas(T) std::byte storage_[N * sizeof(T)];
    std::atomic<size_t> read_idx_{0};
    std::atomic<size_t> write_idx_{0};
    
    T* data() noexcept {
        return std::launder(reinterpret_cast<T*>(storage_));
    }
    
    const T* data() const noexcept {
        return std::launder(reinterpret_cast<const T*>(storage_));
    }
    
    static constexpr size_t advance(size_t idx, size_t step = 1) noexcept {
        return (idx + step) & MASK;
    }
    
public:
    RingBuf() noexcept = default;
    
    ~RingBuf() {
        clear();
    }
    
    // 禁止拷贝
    RingBuf(const RingBuf&) = delete;
    RingBuf& operator=(const RingBuf&) = delete;
    
    // 移动构造
    RingBuf(RingBuf&& other) noexcept(std::is_nothrow_move_constructible_v<T>) 
        : read_idx_(other.read_idx_.load(std::memory_order_acquire))
        , write_idx_(other.write_idx_.load(std::memory_order_acquire))
    {
        // 移动构造时移动已有元素
        const size_t len = other.length();
        size_t other_read_idx = other.read_idx_.load(std::memory_order_relaxed);
        
        for (size_t i = 0; i < len; ++i) {
            size_t idx = advance(other_read_idx, i);
            std::construct_at(data() + idx, std::move(other.data()[idx]));
            std::destroy_at(other.data() + idx);
        }
        other.read_idx_.store(0, std::memory_order_release);
        other.write_idx_.store(0, std::memory_order_release);
    }
    
    // 移动赋值
    RingBuf& operator=(RingBuf&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (this != &other) {
            clear();
            read_idx_.store(other.read_idx_.load(std::memory_order_acquire), std::memory_order_relaxed);
            write_idx_.store(other.write_idx_.load(std::memory_order_acquire), std::memory_order_relaxed);
            
            const size_t len = other.length();
            size_t other_read_idx = other.read_idx_.load(std::memory_order_relaxed);
            
            for (size_t i = 0; i < len; ++i) {
                size_t idx = advance(other_read_idx, i);
                std::construct_at(data() + idx, std::move(other.data()[idx]));
                std::destroy_at(other.data() + idx);
            }
            
            other.read_idx_.store(0, std::memory_order_release);
            other.write_idx_.store(0, std::memory_order_release);
        }
        return *this;
    }
    
    [[nodiscard]] static consteval size_t capacity() noexcept {
        return MAX_CAPACITY;  // 实际可用容量
    }
    
    // 单元素操作
    template<typename... Args>
    [[nodiscard]] size_t try_emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        if (is_full()) return false;
        
        size_t write_idx = write_idx_.load(std::memory_order_acquire);
        std::construct_at(data() + write_idx, std::forward<Args>(args)...);
        write_idx_.store(advance(write_idx), std::memory_order_release);
        return true;
    }
    
    [[nodiscard]] size_t try_pop(T& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (is_empty()) return false;
        
        size_t read_idx = read_idx_.load(std::memory_order_acquire);
        std::construct_at(&value, std::move(data()[read_idx]));
        std::destroy_at(data() + read_idx);
        read_idx_.store(advance(read_idx), std::memory_order_release);
        return true;
    }
    
    [[nodiscard]] T pop_unchecked() noexcept(std::is_nothrow_move_constructible_v<T>) {
        size_t read_idx = read_idx_.load(std::memory_order_acquire);
        T result = std::move(data()[read_idx]);
        std::destroy_at(data() + read_idx);
        read_idx_.store(advance(read_idx), std::memory_order_release);
        return result;
    }
    
    void push_unchecked(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        size_t write_idx = write_idx_.load(std::memory_order_acquire);
        std::construct_at(data() + write_idx, value);
        write_idx_.store(advance(write_idx), std::memory_order_release);
    }
    
    void push_unchecked(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
        size_t write_idx = write_idx_.load(std::memory_order_acquire);
        std::construct_at(data() + write_idx, std::move(value));
        write_idx_.store(advance(write_idx), std::memory_order_release);
    }
    
    [[nodiscard]] size_t try_push(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        if (is_full()) return false;
        push_unchecked(value);
        return true;
    }
    
    [[nodiscard]] size_t try_push(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (is_full()) return false;
        push_unchecked(std::move(value));
        return true;
    }
    
    // 批量操作
    [[nodiscard]] size_t try_push(std::span<const T> src) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        const size_t free_space = free_capacity();
        const size_t to_push = std::min(src.size(), free_space);
        
        if (to_push == 0) return 0;
        
        size_t write_idx = write_idx_.load(std::memory_order_acquire);
        
        // 处理可能的分段写入
        const size_t first_part = std::min(to_push, N - write_idx);
        const size_t second_part = to_push - first_part;
        
        // 构造第一段
        for (size_t i = 0; i < first_part; ++i) {
            std::construct_at(data() + write_idx + i, src[i]);
        }
        
        // 构造第二段（如果需要）
        for (size_t i = 0; i < second_part; ++i) {
            std::construct_at(data() + i, src[first_part + i]);
        }
        
        write_idx_.store(advance(write_idx, to_push), std::memory_order_release);
        return to_push;
    }
    
    [[nodiscard]] size_t try_pop(std::span<T> dst) noexcept(std::is_nothrow_move_constructible_v<T>) {
        const size_t available = length();
        const size_t to_pop = std::min(dst.size(), available);
        
        if (to_pop == 0) return 0;
        
        size_t read_idx = read_idx_.load(std::memory_order_acquire);
        
        // 处理可能的分段读取
        const size_t first_part = std::min(to_pop, N - read_idx);
        const size_t second_part = to_pop - first_part;
        
        // 移动构造并析构第一段
        for (size_t i = 0; i < first_part; ++i) {
            std::construct_at(&dst[i], std::move(data()[read_idx + i]));
            std::destroy_at(data() + read_idx + i);
        }
        
        // 移动构造并析构第二段
        for (size_t i = 0; i < second_part; ++i) {
            std::construct_at(&dst[first_part + i], std::move(data()[i]));
            std::destroy_at(data() + i);
        }
        
        read_idx_.store(advance(read_idx, to_pop), std::memory_order_release);
        return to_pop;
    }
    
    // 查询状态
    [[nodiscard]] size_t length() const noexcept {
        const size_t write_idx = write_idx_.load(std::memory_order_acquire);
        const size_t read_idx = read_idx_.load(std::memory_order_acquire);
        
        if (write_idx >= read_idx) {
            return write_idx - read_idx;
        } else {
            return N - (read_idx - write_idx);
        }
    }
    
    [[nodiscard]] bool is_empty() const noexcept {
        const size_t write_idx = write_idx_.load(std::memory_order_acquire);
        const size_t read_idx = read_idx_.load(std::memory_order_acquire);
        return write_idx == read_idx;
    }
    
    [[nodiscard]] bool is_full() const noexcept {
        // 下一个写入位置等于读位置时表示满了（保留一个空位）
        const size_t next_write = advance(write_idx_.load(std::memory_order_acquire));
        return next_write == read_idx_.load(std::memory_order_acquire);
    }
    
    [[nodiscard]] size_t free_capacity() const noexcept {
        return MAX_CAPACITY - length();
    }
    
    // 管理操作
    void clear() noexcept {
        const size_t len = length();
        size_t read_idx = read_idx_.load(std::memory_order_relaxed);
        
        for (size_t i = 0; i < len; ++i) {
            std::destroy_at(data() + advance(read_idx, i));
        }
        
        read_idx_.store(0, std::memory_order_release);
        write_idx_.store(0, std::memory_order_release);
    }
    
    [[nodiscard]] size_t waste(size_t len) noexcept {
        len = std::min(len, length());
        if (len == 0) return 0;
        
        size_t read_idx = read_idx_.load(std::memory_order_acquire);
        
        for (size_t i = 0; i < len; ++i) {
            std::destroy_at(data() + advance(read_idx, i));
        }
        
        read_idx_.store(advance(read_idx, len), std::memory_order_release);
        return len;
    }
    
};
}