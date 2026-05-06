#pragma once

#include <utility>
#include <type_traits>
#include <functional>
#include <memory>

namespace ymd::heapless{

static constexpr size_t STATIC_FUNCTION_MAX_BUFFER_SIZE = 16;

template <typename Signature, size_t BufferSize = STATIC_FUNCTION_MAX_BUFFER_SIZE>
class Function;

// BufferSize > 0: 通用实现
template <typename Ret, typename... Args, size_t BufferSize>
requires (BufferSize > 0)
class Function<Ret(Args...), BufferSize> {
    alignas(std::max_align_t) std::byte storage_[BufferSize];

    using InvokeFn = Ret(*)(const std::byte*, Args...);
    using DestroyFn = void(*)(std::byte*);

    InvokeFn invoke_ = nullptr;
    DestroyFn destroy_ = nullptr;
public:
    constexpr Function() noexcept = default;

    constexpr ~Function() {
        if (destroy_) destroy_(storage_);
    }

    template <typename F>
    requires (
        std::is_invocable_r_v<Ret, F, Args...> &&
        sizeof(F) <= BufferSize &&
        alignof(F) <= alignof(std::max_align_t)
    )
    constexpr Function(F&& f) noexcept(std::is_nothrow_constructible_v<std::decay_t<F>, F>) {
        using DecayF = std::decay_t<F>;
        std::construct_at(reinterpret_cast<DecayF*>(storage_), std::forward<F>(f));

        invoke_ = [](const std::byte* storage, Args... args) -> Ret {
            return std::invoke(*std::launder(reinterpret_cast<const DecayF*>(storage)), std::forward<Args>(args)...);
        };

        destroy_ = [](std::byte* storage) {
            std::destroy_at(std::launder(reinterpret_cast<DecayF*>(storage)));
        };
    }

    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;

    constexpr Function(Function&& other) noexcept {
        invoke_ = other.invoke_;
        destroy_ = other.destroy_;
        if (invoke_) {
            std::memcpy(storage_, other.storage_, BufferSize);
            other.invoke_ = nullptr;
            other.destroy_ = nullptr;
        }
    }

    constexpr Function& operator=(Function&& other) noexcept {
        if (this != &other) {
            if (destroy_) destroy_(storage_);
            invoke_ = other.invoke_;
            destroy_ = other.destroy_;
            if (invoke_) {
                std::memcpy(storage_, other.storage_, BufferSize);
                other.invoke_ = nullptr;
                other.destroy_ = nullptr;
            }
        }
        return *this;
    }

    constexpr Ret operator()(Args... args) const noexcept {
        return invoke_(storage_, std::forward<Args>(args)...);
    }

    constexpr explicit operator bool() const noexcept {
        return invoke_ != nullptr;
    }

    constexpr bool is_empty() const noexcept{
        return bool(*this);
    }
};

// BufferSize = 0: 退化为函数指针
template <typename Ret, typename... Args>
class Function<Ret(Args...), 0> {
    using FnPtr = Ret(*)(Args...);
    FnPtr fn_ = nullptr;

public:
    constexpr Function() noexcept = default;

    constexpr Function(FnPtr fn) noexcept : fn_(fn) {}

    template <typename F>
    requires (std::is_convertible_v<F, FnPtr>)
    constexpr Function(F&& f) noexcept : fn_(std::forward<F>(f)) {}

    constexpr Ret operator()(Args... args) const noexcept {
        return fn_(std::forward<Args>(args)...);
    }

    constexpr explicit operator bool() const noexcept {
        return fn_ != nullptr;
    }

    constexpr bool is_empty() const noexcept{
        return bool(*this);
    }
};

}

namespace ymd{
template <typename Signature, size_t BufferSize = heapless::STATIC_FUNCTION_MAX_BUFFER_SIZE>
using HeaplessFunction = heapless::Function<Signature, BufferSize>;
}