#pragma once

#include <utility>       // std::forward, std::move
#include <type_traits>   // std::aligned_storage, std::is_invocable_r
#include <functional>    // std::invoke

namespace ymd::heapless{
static constexpr size_t STATIC_FUNCRION_MAX_BUFFER_SIZE = 16;
template <typename Signature, size_t BufferSize = STATIC_FUNCRION_MAX_BUFFER_SIZE>
class Function;

template <typename Ret, typename... Args, size_t BufferSize>
class Function<Ret(Args...), BufferSize> {
    // 存储可调用对象的缓冲区（对齐）
    using Storage = std::aligned_storage_t<BufferSize>;
    Storage storage_;

    // 类型擦除的分发函数指针
    using InvokeFn = Ret(*)(const Storage&, Args&&...);
    InvokeFn invoke_ = nullptr;
public:
    // 默认构造（空函数）
    constexpr Function() noexcept 
        : invoke_(nullptr) {}

    // 从可调用对象构造（lambda、函数指针等）
    template <typename F>
    requires (
        std::is_invocable_r_v<Ret, F, Args...> &&
        (sizeof(F) <= BufferSize) &&
        std::is_nothrow_move_constructible_v<F>
    )
    constexpr Function(F&& f) noexcept {
        // 编译期检查缓冲区大小
        static_assert(sizeof(F) <= BufferSize, 
            "Callable too large for Function buffer!");

        // 存储可调用对象
        new (&storage_) F(std::forward<F>(f));

        // 设置分发函数
        invoke_ = [](const Storage& storage, Args&&... args) -> Ret {
            return std::invoke(
                *reinterpret_cast<const F*>(&storage),
                std::forward<Args>(args)...
            );
        };
    }

    // 调用运算符
    constexpr Ret operator()(Args... args) const {
        return invoke_(storage_, std::forward<Args>(args)...);
    }

    // 检查是否非空
    constexpr explicit operator bool() const noexcept {
        return invoke_ != nullptr;
    }
};

}

namespace ymd{

// template <typename Ret, typename... Args, size_t BufferSize>
// using HeaplessFunction = heapless::Function<Ret(Args...), BufferSize>;
}