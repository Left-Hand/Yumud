#pragma once

#include "lib/lib_o1heap.hpp"
#include <memory>
#include <type_traits>
#include <memory_resource>
#include <span>

#if 0
namespace ymd::mem::o1heap {
    
// 前置声明，因为我们需要在工厂函数中使用它
class [[nodiscard]] HeapManager {
private:
    lib_o1heap::O1HeapInstance* instance_;
    
public:
    explicit HeapManager(std::span<uint8_t> buffer) : instance_(nullptr) {
        if (buffer.size() >= lib_o1heap::o1heapMinArenaSize) {
            instance_ = lib_o1heap::o1heapInit(buffer.data(), buffer.size());
        }
    }
    
    // Note: According to original library design, explicit cleanup is usually not needed
    // However, we're adding a virtual destructor to make it clear this isn't designed for inheritance
    ~HeapManager() = default;
    
    // Prevent copying and moving since the heap instance is tied to specific memory
    HeapManager(const HeapManager&) = delete;
    HeapManager& operator=(const HeapManager&) = delete;

    lib_o1heap::O1HeapInstance* get_instance() const noexcept {
        return instance_;
    }
    
    bool is_valid() const noexcept {
        return instance_ && instance_->o1heapDoInvariantsHold();
    }
    
    const lib_o1heap::O1HeapDiagnostics & get_diagnostics() const {
        return instance_->diagnostics;
    }
};

#if 0

#endif

// PMR 内存资源包装器
class [[nodiscard]] O1HeapMemoryResource : public std::pmr::memory_resource {
private:
    std::span<uint8_t> buffer_;  // Note: User must ensure buffer lifetime exceeds resource lifetime
    HeapManager heap_manager_;
    
public:
    explicit O1HeapMemoryResource(std::span<uint8_t> buffer) 
        : buffer_(buffer), heap_manager_(buffer) {}
    
    // 实现 do_allocate 方法
    [[nodiscard]] void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        // 检查是否满足对齐要求
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {  // Use standard default alignment instead of max_align_t
            // For now, reject over-aligned allocations, but consider logging this event
            return nullptr;  // 不支持特殊对齐
        }
        
        if (!heap_manager_.is_valid()) {
            return nullptr;
        }
        
        // 使用 o1heap 分配内存
        void* ptr = lib_o1heap::o1heapAllocate(
            heap_manager_.get_instance(), 
            bytes
        );
        
        if (!ptr) {
            // Consider throwing std::bad_alloc here or returning nullptr based on requirements
            return nullptr;
        }
        
        return ptr;
    }
    
    // 实现 do_deallocate 方法
    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
        (void)bytes;      // 忽略大小参数
        (void)alignment;  // 忽略对齐参数
        
        if (p != nullptr && heap_manager_.is_valid()) {
            lib_o1heap::o1heapFree(
                heap_manager_.get_instance(), 
                p
            );
        }
    }
    
    // 实现 do_is_equal 方法
    [[nodiscard]] bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
    
    // 获取诊断信息
    const lib_o1heap::O1HeapDiagnostics & get_diagnostics() const {
        return heap_manager_.get_diagnostics();
    }
    
    [[nodiscard]] bool is_valid() const noexcept {
        return heap_manager_.is_valid();
    }
    
    // Provide access to the heap manager if needed
    const HeapManager& get_heap_manager() const noexcept {
        return heap_manager_;
    }
};


// 工厂函数：从内存缓冲区创建管理器
inline HeapManager make_heap_manager(std::span<uint8_t> buffer) {
    return HeapManager(buffer);
}

// Alternative factory function that returns a shared_ptr-based resource
// where the user doesn't have to manage buffer lifetime separately
inline std::unique_ptr<O1HeapMemoryResource> make_memory_resource(std::vector<uint8_t>& buffer) {
    return std::make_unique<O1HeapMemoryResource>(std::span<uint8_t>{buffer});
}

} // namespace ymd::mem::o1heap

#endif

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