#pragma once

#include "lib/lib_o1heap.hpp"
#include <memory>
#include <type_traits>
#include <span>

namespace ymd::mem::o1heap{
template<typename T>
class [[nodiscard]] O1HeapAllocator {
public:
    using Self = O1HeapAllocator<T>;
    using O1HeapInstance = lib_o1heap::O1HeapInstance;
    // Type aliases required by the allocator concept
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Constructor that accepts a heap instance
    explicit O1HeapAllocator(O1HeapInstance & heap) noexcept 
        : inst_(heap) {}

    static inline Option<Self> try_from_buf(std::span<uint8_t> buffer_bytes){
        auto* instance = lib_o1heap::o1heapInit(buffer_bytes.data(), buffer_bytes.size());
        if (instance == nullptr) return None;
        return Some(Self(*instance));
    }

    
    // Copy constructor
    template<typename U>
    O1HeapAllocator(const O1HeapAllocator<U>& other) noexcept 
        : inst_(other.heap_instance()) {}

    // Destructor
    ~O1HeapAllocator() = default;

    // Get the underlying heap instance
    O1HeapInstance & heap_instance() const noexcept {
        return inst_;
    }

    const auto & diagnostics() const noexcept {
        return inst_.diagnostics;
    }

    // Equality operator
    template<typename U>
    bool operator==(const O1HeapAllocator<U>& other) const noexcept {
        return inst_ == other.heap_instance();
    }

    // Inequality operator
    template<typename U>
    bool operator!=(const O1HeapAllocator<U>& other) const noexcept {
        return !(*this == other);
    }

    // Allocate memory
    pointer allocate(size_type n) noexcept {
        if (n == 0) {
            return nullptr;
        }

        // Check for potential overflow
        if (n > max_size()) {
            __builtin_trap();
        }

        void* ptr = inst_.o1heapAllocate(n * sizeof(T));
        if (!ptr) {
            __builtin_trap();
        }

        return static_cast<pointer>(ptr);
    }

    // Deallocate memory
    void deallocate(pointer p, size_type n) noexcept {
        if (p != nullptr) [[likely]]{
            inst_.o1heapFree(p);
        }
        (void)n; // Suppress unused parameter warning
    }

    // Construct an object
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new(p) U(std::forward<Args>(args)...);
    }

    // Destroy an object
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }

    // Maximum size that can be allocated
    size_type max_size() const noexcept {
        return inst_.o1heapGetMaxAllocationSize() / sizeof(T);
    }

    // Create an allocator for a different type
    template<typename U>
    O1HeapAllocator<U> rebind_to() const noexcept {
        return O1HeapAllocator<U>(inst_);
    }

private:
    O1HeapInstance & inst_;
};


// 工厂函数：从内存缓冲区创建分配器
template<typename T = uint8_t>
Option<O1HeapAllocator<T>> make_o1heap_allocator(std::span<T> buffer) {
    return O1HeapAllocator<T>::try_from_buf(buffer);
}

}