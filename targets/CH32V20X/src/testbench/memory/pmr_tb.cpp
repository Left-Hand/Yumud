#include <memory_resource>
#include <cstdint>
#include <array>

#include <memory>
#include <type_traits>
#include <utility>
#include <cstring>

// 小对象优化缓冲区大小
static constexpr size_t FUNCTION_SBO_SIZE = 32;

// 替换异常的错误处理策略
enum class FunctionError {
    SUCCESS,
    BAD_FUNCTION_CALL,
    ALLOCATION_FAILED
};


template<typename> class Function;

template<typename R, typename... Args>
class Function<R(Args...)>
{
private:
    // 可调用对象接口
    struct CallableBase
    {
        virtual R invoke(Args... args) noexcept = 0;
        virtual CallableBase* clone(void* where) const noexcept = 0;
        virtual void destroy(bool use_sbo) noexcept = 0;
        virtual const void* get_target() const noexcept = 0;
        virtual bool is_type(const std::type_info&) const noexcept = 0;
    };

    // 具体可调用对象的实现
    template<typename F>
    struct Callable : CallableBase
    {
        F f;

        template<typename G>
        Callable(G&& func) noexcept : f(std::forward<G>(func)) {}

        R invoke(Args... args) noexcept override
        {
            return f(std::forward<Args>(args)...);
        }

        CallableBase* clone(void* where) const noexcept override
        {
            return std::construct_at(static_cast<Callable<F>*>(where), f);
        }

        void destroy(bool use_sbo) noexcept override
        {
            if (!use_sbo) {
                std::destroy_at(this);
                // 内存释放由外部处理
            } else {
                std::destroy_at(this);
            }
        }

        const void* get_target() const noexcept override
        {
            return &f;
        }

    };

    // 分配器接口
    struct AllocatorBase
    {
        virtual void* allocate(size_t size) noexcept = 0;
        virtual void deallocate(void* ptr, size_t size) noexcept = 0;
        virtual AllocatorBase* clone() const noexcept = 0;
        virtual void destroy() noexcept = 0;
    };

    // 具体分配器实现
    template<typename A>
    struct AllocatorImpl 
    {
        A alloc;

        AllocatorImpl(const A& a) noexcept : alloc(a) {}
        AllocatorImpl(A&& a) noexcept : alloc(std::move(a)) {}

        void* allocate(size_t size) noexcept override
        {
            return std::allocator_traits<A>::allocate(alloc, size);
        }

        void deallocate(void* ptr, size_t size) noexcept override
        {
            std::allocator_traits<A>::deallocate(alloc, 
                static_cast<typename std::allocator_traits<A>::pointer>(ptr), 
                size);
        }

        AllocatorBase* clone() const noexcept override
        {
            void* mem = ::operator new(sizeof(AllocatorImpl<A>));
            return std::construct_at(static_cast<AllocatorImpl<A>*>(mem), alloc);
        }

        void destroy() noexcept override
        {
            std::destroy_at(this);
            ::operator delete(this);
        }
    };

    // 使用小对象优化缓冲区
    alignas(alignof(void*)) mutable char storage[FUNCTION_SBO_SIZE];
    CallableBase* callable = nullptr;
    AllocatorBase* allocator = nullptr;
    bool use_sbo = false;

    // 错误处理
    static void handle_error(FunctionError error) noexcept
    {
    }

public:
    // 默认构造函数
    Function() noexcept = default;

    // 从可调用对象构造
    template<typename F, typename A = std::allocator<char>>
    Function(F&& f, const A& alloc = A()) noexcept
    {
        constexpr size_t size = sizeof(Callable<std::decay_t<F>>);
        
        if (size <= FUNCTION_SBO_SIZE && 
            alignof(Callable<std::decay_t<F>>) <= alignof(decltype(storage)))
        {
            // 使用小对象优化
            callable = std::construct_at(
                reinterpret_cast<Callable<std::decay_t<F>>*>(storage), 
                std::forward<F>(f)
            );
            use_sbo = true;
        }
        else
        {
            // 使用分配器分配内存
            allocator = std::construct_at(
                reinterpret_cast<AllocatorImpl<A>*>(::operator new(sizeof(AllocatorImpl<A>))),
                alloc
            );
            
            void* mem = allocator->allocate(size);
            if (!mem) {
                handle_error(FunctionError::ALLOCATION_FAILED);
                allocator->destroy();
                allocator = nullptr;
                return;
            }
            
            callable = std::construct_at(
                reinterpret_cast<Callable<std::decay_t<F>>*>(mem), 
                std::forward<F>(f)
            );
            use_sbo = false;
        }
    }

    // 拷贝构造函数
    Function(const Function& other) noexcept
    {
        if (!other.callable) return;

        if (other.allocator) {
            allocator = other.allocator->clone();
            if (!allocator) {
                handle_error(FunctionError::ALLOCATION_FAILED);
                return;
            }
        }
        
        if (other.use_sbo) {
            callable = other.callable->clone(storage);
            use_sbo = true;
        } else {
            const size_t size = sizeof(decltype(*other.callable));
            void* mem = allocator->allocate(size);
            if (!mem) {
                handle_error(FunctionError::ALLOCATION_FAILED);
                if (allocator) {
                    allocator->destroy();
                    allocator = nullptr;
                }
                return;
            }
            callable = other.callable->clone(mem);
            use_sbo = false;
        }
    }

    // 移动构造函数
    Function(Function&& other) noexcept
    {
        swap(other);
    }

    // 析构函数
    ~Function() noexcept
    {
        reset();
        if (allocator) {
            allocator->destroy();
        }
    }

    // 赋值运算符
    Function& operator=(const Function& other) noexcept
    {
        if (this != &other) {
            Function temp(other);
            swap(temp);
        }
        return *this;
    }

    Function& operator=(Function&& other) noexcept
    {
        if (this != &other) {
            reset();
            swap(other);
        }
        return *this;
    }

    // 交换
    void swap(Function& other) noexcept
    {
        std::swap(callable, other.callable);
        std::swap(allocator, other.allocator);
        std::swap(use_sbo, other.use_sbo);
        
        // 交换小对象缓冲区内容
        if (use_sbo && other.use_sbo) {
            alignas(alignof(void*)) char temp[FUNCTION_SBO_SIZE];
            std::memcpy(temp, storage, FUNCTION_SBO_SIZE);
            std::memcpy(storage, other.storage, FUNCTION_SBO_SIZE);
            std::memcpy(other.storage, temp, FUNCTION_SBO_SIZE);
        }
        else if (use_sbo) {
            // 当前使用SBO，对方不使用
            alignas(alignof(void*)) char temp[FUNCTION_SBO_SIZE];
            std::memcpy(temp, storage, FUNCTION_SBO_SIZE);
            callable = other.callable;
            other.callable = reinterpret_cast<CallableBase*>(other.storage);
            std::memcpy(other.storage, temp, FUNCTION_SBO_SIZE);
            std::swap(use_sbo, other.use_sbo);
        }
        else if (other.use_sbo) {
            // 对方使用SBO，当前不使用
            other.swap(*this);
        }
    }

    // 重置函数对象
    void reset() noexcept
    {
        if (callable) {
            callable->destroy(use_sbo);
            if (!use_sbo && allocator) {
                allocator->deallocate(callable, sizeof(decltype(*callable)));
            }
            callable = nullptr;
        }
        use_sbo = false;
    }

    // 调用运算符（无异常版本）
    R operator()(Args... args) const noexcept
    {
        if (!callable) {
            handle_error(FunctionError::BAD_FUNCTION_CALL);
            if constexpr (!std::is_void_v<R>) {
                return R{};
            }
        }
        return callable->invoke(std::forward<Args>(args)...);
    }

    // 检查是否包含可调用对象
    explicit operator bool() const noexcept
    {
        return callable != nullptr;
    }

    template<typename T>
    T* target() noexcept
    {
        return const_cast<T*>(static_cast<const Function*>(this)->target<T>());
    }
};

// 特化用于空函数的比较
template<typename R, typename... Args>
bool operator==(const Function<R(Args...)>& f, std::nullptr_t) noexcept
{
    return !f;
}

template<typename R, typename... Args>
bool operator==(std::nullptr_t, const Function<R(Args...)>& f) noexcept
{
    return !f;
}

template<typename R, typename... Args>
bool operator!=(const Function<R(Args...)>& f, std::nullptr_t) noexcept
{
    return static_cast<bool>(f);
}

template<typename R, typename... Args>
bool operator!=(std::nullptr_t, const Function<R(Args...)>& f) noexcept
{
    return static_cast<bool>(f);
}

[[maybe_unused]] static void pmr_tb(){
    // 在栈上分配一些内存：
    std::array<std::byte, 200000> buf;

    // 将它用作vector的初始内存池：
    std::pmr::monotonic_buffer_resource pool{buf.data(), buf.size()};
    // std::pmr::vector<std::string> coll{&pool};
    // std::pmr::function<void(void)> coll{&pool};
    // std::pmr::
}