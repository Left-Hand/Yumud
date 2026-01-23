#pragma once

#include "lib/lib_o1heap.hpp"
#include <memory>
#include <type_traits>
#include <memory_resource>
#include <span>

namespace ymd::mem::o1heap {
    
// 前置声明，因为我们需要在工厂函数中使用它
class HeapManager {
private:
    lib_o1heap::O1HeapInstance* instance_;
    
public:
    explicit HeapManager(std::span<uint8_t> buffer) : instance_(nullptr) {
        if (buffer.size() >= lib_o1heap::o1heapMinArenaSize) {
            instance_ = lib_o1heap::o1heapInit(buffer.data(), buffer.size());
        }
    }
    
    ~HeapManager() = default;  // 注意：根据原始库设计，通常不需要显式清理
    
    lib_o1heap::O1HeapInstance* get_instance() const noexcept {
        return instance_;
    }
    
    bool is_valid() const noexcept {
        return instance_ && lib_o1heap::o1heapDoInvariantsHold(instance_);
    }
    
    lib_o1heap::O1HeapDiagnostics get_diagnostics() const {
        if (instance_) {
            return lib_o1heap::o1heapGetDiagnostics(instance_);
        }
        return {};
    }
};

template<typename T>
class O1HeapAllocator {
public:
    using O1HeapInstance = lib_o1heap::O1HeapInstance;
    // Type aliases required by the allocator concept
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Rebind allocator to type U
    template<typename U>
    struct rebind {
        using other = O1HeapAllocator<U>;
    };

    // Default constructor
    O1HeapAllocator() noexcept : heap_instance_(nullptr) {}

    // Constructor that accepts a heap instance
    explicit O1HeapAllocator(O1HeapInstance* heap) noexcept 
        : heap_instance_(heap) {}

    // Constructor that accepts a HeapManager
    explicit O1HeapAllocator(const HeapManager& manager) noexcept
        : heap_instance_(manager.get_instance()) {}
    
    // Copy constructor
    template<typename U>
    O1HeapAllocator(const O1HeapAllocator<U>& other) noexcept 
        : heap_instance_(other.get_heap_instance()) {}

    // Destructor
    ~O1HeapAllocator() = default;

    // Get the underlying heap instance
    O1HeapInstance* get_heap_instance() const noexcept {
        return heap_instance_;
    }

    // Equality operator
    template<typename U>
    bool operator==(const O1HeapAllocator<U>& other) const noexcept {
        return heap_instance_ == other.get_heap_instance();
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
        
        if (!heap_instance_) {
            return nullptr; // Or handle error appropriately
        }

        // Check for potential overflow
        if (n > max_size()) {
            return nullptr;
        }

        void* ptr = lib_o1heap::o1heapAllocate(heap_instance_, n * sizeof(T));
        if (!ptr) {
            return nullptr;
        }

        return static_cast<pointer>(ptr);
    }

    // Deallocate memory
    void deallocate(pointer p, size_type n) noexcept {
        if (p != nullptr) {
            lib_o1heap::o1heapFree(heap_instance_, p);
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
        if (!heap_instance_) {
            return 0;
        }
        return lib_o1heap::o1heapGetMaxAllocationSize(heap_instance_) / sizeof(T);
    }

    // Create an allocator for a different type
    template<typename U>
    O1HeapAllocator<U> rebind_to() const noexcept {
        return O1HeapAllocator<U>(heap_instance_);
    }

private:
    O1HeapInstance* heap_instance_;
};

// PMR 内存资源包装器
class O1HeapMemoryResource : public std::pmr::memory_resource {
private:
    std::span<uint8_t> buffer_;
    HeapManager heap_manager_;
    
public:
    explicit O1HeapMemoryResource(std::span<uint8_t> buffer) 
        : buffer_(buffer), heap_manager_(buffer) {}
    
    // 实现 do_allocate 方法
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        // 检查是否满足对齐要求
        if (alignment > alignof(std::max_align_t)) {
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
            return nullptr;
        }
        
        return ptr;
    }
    
    // 实现 do_deallocate 方法
    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
        (void)bytes;      // 忽略大小参数
        (void)alignment;  // 忽略对齐参数
        
        if (p != nullptr) {
            lib_o1heap::o1heapFree(
                heap_manager_.get_instance(), 
                p
            );
        }
    }
    
    // 实现 do_is_equal 方法
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
    
    // 获取诊断信息
    lib_o1heap::O1HeapDiagnostics get_diagnostics() const {
        return heap_manager_.get_diagnostics();
    }
    
    bool is_valid() const noexcept {
        return heap_manager_.is_valid();
    }
};

// 工厂函数：从内存缓冲区创建分配器
template<typename T = int8_t>
O1HeapAllocator<T> make_o1heap_allocator(std::span<uint8_t> buffer) {
    auto* instance = lib_o1heap::o1heapInit(buffer.data(), buffer.size());
    if (!instance) {
        return O1HeapAllocator<T>(); // 返回无效分配器
    }
    return O1HeapAllocator<T>(instance);
}

// 工厂函数：从内存缓冲区创建管理器
inline HeapManager make_heap_manager(std::span<uint8_t> buffer) {
    return HeapManager(buffer);
}

// 工厂函数：使用管理器对象创建分配器
template<typename T = int8_t>
O1HeapAllocator<T> make_o1heap_allocator_with_manager(const HeapManager& manager) {
    return O1HeapAllocator<T>(manager);
}


} // namespace ymd::mem::o1heap