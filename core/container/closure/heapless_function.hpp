#pragma once

#include <utility>
#include <type_traits>
#include <functional>
#include <memory>

namespace ymd::heapless{

static constexpr size_t STATIC_FUNCTION_MAX_BUFFER_SIZE = 16;

template <typename Signature, size_t BUFFER_SIZE = STATIC_FUNCTION_MAX_BUFFER_SIZE>
class Function;



// BUFFER_SIZE = 0: 退化为函数指针
template <typename Ret, typename... Args>
class Function<Ret(Args...), 0> {
    using FnPtr = Ret(*)(Args...);
    FnPtr fn_ = nullptr;

public:
    Function() noexcept = default;

    Function(FnPtr fn) noexcept : fn_(fn) {}

    template <typename F>
    requires (std::is_convertible_v<F, FnPtr>)
    constexpr Function(F&& f) noexcept : fn_(std::forward<F>(f)) {}

    constexpr Ret operator()(Args... args) const noexcept {
        return fn_(std::forward<Args>(args)...);
    }

    constexpr explicit operator bool() const noexcept {
        return fn_ != nullptr;
    }

    constexpr bool is_null() const noexcept{
        return bool(*this);
    }
};


// BUFFER_SIZE > 0: 通用实现
template <typename Ret, typename... Args, size_t BUFFER_SIZE>
requires (BUFFER_SIZE > 0)
class Function<Ret(Args...), BUFFER_SIZE> {
    alignas(std::max_align_t) std::byte storage_[BUFFER_SIZE];

    using InvokeFn = Ret(*)(const std::byte*, Args...);
    using DestroyFn = void(*)(std::byte*);

    InvokeFn invoke_ = nullptr;
    DestroyFn destroy_ = nullptr;
public:
    Function() noexcept = default;

    ~Function() {
        if (destroy_) destroy_(storage_);
    }

    template <typename F>
    requires (
        std::is_invocable_r_v<Ret, F, Args...> &&
        sizeof(F) <= BUFFER_SIZE &&
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

    Function(Function&& other) noexcept {
        invoke_ = other.invoke_;
        destroy_ = other.destroy_;
        if (invoke_) {
            memcpy(storage_, other.storage_, BUFFER_SIZE);
            other.invoke_ = nullptr;
            other.destroy_ = nullptr;
        }
    }

    Function& operator=(Function&& other) noexcept {
        if (this != &other) {
            if (destroy_) destroy_(storage_);
            invoke_ = other.invoke_;
            destroy_ = other.destroy_;
            if (invoke_) {
                memcpy(storage_, other.storage_, BUFFER_SIZE);
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

    constexpr bool is_null() const noexcept{
        return bool(*this);
    }
};

}

namespace ymd{
template <typename Signature, size_t BUFFER_SIZE = heapless::STATIC_FUNCTION_MAX_BUFFER_SIZE>
using HeaplessFunction = heapless::Function<Signature, BUFFER_SIZE>;
}