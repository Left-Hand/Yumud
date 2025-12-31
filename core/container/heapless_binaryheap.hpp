#pragma once

#include <cstdint>
#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "core/utils/result.hpp"
#include "core/utils/Option.hpp"


namespace ymd::heapless {

// 前向声明
template<typename T, size_t N, typename Compare = std::less<T>>
class BinaryHeap;

// 迭代器实现
template<typename T, size_t N, typename Compare>
class BinaryHeapIterator {
private:
    const BinaryHeap<T, N, Compare>* heap_;
    size_t index_;
    
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = const T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;
    
    BinaryHeapIterator(const BinaryHeap<T, N, Compare>* heap, size_t index)
        : heap_(heap), index_(index) {}
    
    reference operator*() const {
        return (*heap_)[index_];
    }
    
    pointer operator->() const {
        return &(*heap_)[index_];
    }
    
    BinaryHeapIterator& operator++() {
        ++index_;
        return *this;
    }
    
    BinaryHeapIterator operator++(int) {
        BinaryHeapIterator temp = *this;
        ++index_;
        return temp;
    }
    
    bool operator==(const BinaryHeapIterator& other) const {
        return heap_ == other.heap_ && index_ == other.index_;
    }
    
    bool operator!=(const BinaryHeapIterator& other) const {
        return !(*this == other);
    }
};

// BinaryHeap 主类
template<typename T, size_t N, typename Compare>
class BinaryHeap {
    static_assert(N > 0, "Capacity must be greater than 0");
    using Self = BinaryHeap;
private:
    T data_[N];
    size_t len_;
    Compare cmp_;
    
    // 获取父节点索引
    constexpr size_t parent(size_t i) const noexcept { return (i - 1) / 2; }
    
    // 获取左子节点索引
    constexpr size_t left_child(size_t i) const noexcept { return 2 * i + 1; }
    
    // 获取右子节点索引
    constexpr size_t right_child(size_t i) const noexcept { return 2 * i + 2; }
    
    // 上浮操作
    void sift_up(size_t i) noexcept {
        while (i > 0) {
            size_t p = parent(i);
            if (cmp_(data_[p], data_[i])) {
                std::swap(data_[p], data_[i]);
                i = p;
            } else {
                break;
            }
        }
    }
    
    // 下沉操作
    void sift_down(size_t i) noexcept {
        while (true) {
            size_t largest = i;
            size_t l = left_child(i);
            size_t r = right_child(i);
            
            if (l < len_ && cmp_(data_[largest], data_[l])) {
                largest = l;
            }
            
            if (r < len_ && cmp_(data_[largest], data_[r])) {
                largest = r;
            }
            
            if (largest != i) {
                std::swap(data_[i], data_[largest]);
                i = largest;
            } else {
                break;
            }
        }
    }
    
    // 重建堆
    void rebuild_heap() noexcept {
        if (len_ > 1) {
            for (size_t i = (len_ - 2) / 2; i > 0; --i) {
                sift_down(i);
            }
            sift_down(0);
        }
    }
    
public:
    // 类型别名
    using value_type = T;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using iterator = BinaryHeapIterator<T, N, Compare>;
    using const_iterator = BinaryHeapIterator<T, N, Compare>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    
    // 构造函数
    constexpr BinaryHeap() noexcept : len_(0), cmp_() {}
    
    explicit constexpr BinaryHeap(const Compare& comp) noexcept 
        : len_(0), cmp_(comp) {}
    
    template<typename InputIt>
    explicit BinaryHeap(InputIt first, InputIt last, const Compare& comp = Compare())
        : len_(0), cmp_(comp) {
        for (; first != last && len_ < N; ++first) {
            data_[len_++] = *first;
        }
        make_heap();
    }
    
    explicit BinaryHeap(std::initializer_list<T> init, const Compare& comp = Compare())
        : len_(0), cmp_(comp) {
        for (const auto& item : init) {
            if (len_ < N) {
                data_[len_++] = item;
            } else {
                break;
            }
        }
        make_heap();
    }
    
    // 拷贝构造函数
    BinaryHeap(const BinaryHeap& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
        : len_(other.len_), cmp_(other.cmp_) {
        for (size_t i = 0; i < len_; ++i) {
            data_[i] = other.data_[i];
        }
    }
    
    // 移动构造函数
    BinaryHeap(BinaryHeap&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
        : len_(other.len_), cmp_(std::move(other.cmp_)) {
        for (size_t i = 0; i < len_; ++i) {
            data_[i] = std::move(other.data_[i]);
        }
        other.len_ = 0;
    }
    
    // 赋值运算符
    BinaryHeap& operator=(const BinaryHeap& other) noexcept(std::is_nothrow_copy_assignable<T>::value) {
        if (this != &other) {
            len_ = other.len_;
            cmp_ = other.cmp_;
            for (size_t i = 0; i < len_; ++i) {
                data_[i] = other.data_[i];
            }
        }
        return *this;
    }
    
    BinaryHeap& operator=(BinaryHeap&& other) noexcept(std::is_nothrow_move_assignable<T>::value) {
        if (this != &other) {
            len_ = other.len_;
            cmp_ = std::move(other.cmp_);
            for (size_t i = 0; i < len_; ++i) {
                data_[i] = std::move(other.data_[i]);
            }
            other.len_ = 0;
        }
        return *this;
    }
    
    // 容量相关
    constexpr bool is_empty() const noexcept { return len_ == 0; }
    constexpr bool is_full() const noexcept { return len_ == N; }
    constexpr size_t capacity() const noexcept { return N; }
    constexpr size_t length() const noexcept { return len_; }
    
    // 元素访问
    const_reference peek() const {
        if (is_empty()) {
            PANIC("Heap is empty");
        }
        return data_[0];
    }
    
    
    void push_unchecked(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value) {
        data_[len_] = std::move(value);
        sift_up(len_);
        ++len_;
    }

    Result<void, void> push(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value) {
        if(is_full()) return Err();
        push_unchecked(std::move(value));
        return Ok();
    }
    
    template<typename... Args>
    void emplace_unchecked(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) {
        data_[len_] = T(std::forward<Args>(args)...);
        sift_up(len_);
        ++len_;
    }

    template<typename... Args>
    Result<void, void> emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) { 
        if(is_full()) return Err();
        emplace_unchecked(std::forward<Args>(args)...);
        return Ok();
    }
    
    T pop_unchecked() {
        if (is_empty()) {
            PANIC("Heap is empty");
        }
        T result = std::move(data_[0]);
        data_[0] = std::move(data_[len_ - 1]);
        --len_;
        if (len_ > 0) {
            sift_down(0);
        }
        return result;
    }
    
    Option<T> pop() noexcept {
        if (is_empty())
            return None;
        return Some(pop_unchecked());
    }
    
    
    bool try_push(const T& value) noexcept(std::is_nothrow_copy_constructible<T>::value) {
        return push(value).is_ok();
    }
    
    bool try_push(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value) {
        return push(std::move(value)).is_ok();
    }
    
    // 清空堆
    void clear() noexcept {
        len_ = 0;
    }
    
    // 重建堆
    void make_heap() noexcept {
        if (len_ > 1) {
            for (size_t i = (len_ - 2) / 2; i > 0; --i) {
                sift_down(i);
            }
            sift_down(0);
        }
    }
    
    // 保留容量内的元素
    void retain(std::function<bool(const T&)> f) {
        size_t write_idx = 0;
        for (size_t read_idx = 0; read_idx < len_; ++read_idx) {
            if (f(data_[read_idx])) {
                if (write_idx != read_idx) {
                    data_[write_idx] = std::move(data_[read_idx]);
                }
                ++write_idx;
            }
        }
        len_ = write_idx;
        make_heap();
    }
    
    // 迭代器支持
    iterator begin() { return iterator(this, 0); }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator cbegin() const { return const_iterator(this, 0); }
    
    iterator end() { return iterator(this, len_); }
    const_iterator end() const { return const_iterator(this, len_); }
    const_iterator cend() const { return const_iterator(this, len_); }
    
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
    
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }
    
    // 元素访问（不安全，用于迭代器）
    const_reference operator[](size_t index) const {
        return data_[index];
    }
    
    // 比较运算符
    template<size_t M, typename OtherCompare>
    bool operator==(const BinaryHeap<T, M, OtherCompare>& other) const {
        if (len_ != other.length()) {
            return false;
        }
        auto temp1 = this->into_sorted_vec();
        auto temp2 = other.into_sorted_vec();
        return temp1 == temp2;
    }
    
    template<size_t M, typename OtherCompare>
    bool operator!=(const BinaryHeap<T, M, OtherCompare>& other) const {
        return !(*this == other);
    }
    
    // 交换
    void swap(BinaryHeap& other) noexcept {
        using std::swap;
        size_t min_len = std::min(len_, other.len_);
        size_t max_len = std::max(len_, other.len_);
        
        // 交换较小长度范围内的元素
        for (size_t i = 0; i < min_len; ++i) {
            swap(data_[i], other.data_[i]);
        }
        
        // 如果长度不同，移动剩余元素
        if (len_ < other.len_) {
            for (size_t i = len_; i < other.len_; ++i) {
                data_[i] = std::move(other.data_[i]);
            }
        } else if (other.len_ < len_) {
            for (size_t i = other.len_; i < len_; ++i) {
                other.data_[i] = std::move(data_[i]);
            }
        }
        
        swap(len_, other.len_);
        swap(cmp_, other.cmp_);
        
        // 重建两个堆
        make_heap();
        other.make_heap();
    }
    
    friend OutputStream & operator<<(OutputStream & os, const Self & self) { 
        TODO();
        return os;
        // os << "BinaryHeap[";
        // for (size_t i = 0; i < len_; ++i) {
        //     if (i > 0) os << ", ";
        //     os << data_[i];
        // }
        // os << "] (length=" << len_ << ", cap=" << N << ")\n";
    }
};

// 辅助函数：创建 BinaryHeap
template<typename T, size_t N, typename Compare = std::less<T>>
constexpr BinaryHeap<T, N, Compare> make_binary_heap() {
    return BinaryHeap<T, N, Compare>();
}

template<typename T, size_t N, typename Compare = std::less<T>>
BinaryHeap<T, N, Compare> from_iter(const T* first, const T* last, Compare comp = Compare()) {
    return BinaryHeap<T, N, Compare>(first, last, comp);
}

template<typename T, size_t N, typename Compare = std::less<T>>
BinaryHeap<T, N, Compare> from_slice(const T (&arr)[N], Compare comp = Compare()) {
    return BinaryHeap<T, N, Compare>(arr, arr + N, comp);
}

// 全局 swap 函数
template<typename T, size_t N, typename Compare>
void swap(BinaryHeap<T, N, Compare>& lhs, BinaryHeap<T, N, Compare>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace heapless

namespace ymd{
template<typename T, size_t N, typename Compare = std::less<T>>
using HeaplessBinaryHeap = heapless::BinaryHeap<T, N, Compare>;
}