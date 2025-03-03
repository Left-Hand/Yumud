#pragma once

#include <optional>
#include <type_traits>

namespace ymd {

// Helper template to unwrap optional types
template <typename T>
struct __unwrap_helper {
};

// Specialization for std::optional
template <typename T>
struct __unwrap_helper<std::optional<T>> {
    // Unwrap a non-const rvalue optional
    static constexpr T&& unwrap(std::optional<T>&& opt) {
        return std::move(opt.value());
    }

    // Unwrap a const lvalue optional
    static constexpr const T& unwrap(const std::optional<T>& opt) {
        return opt.value();
    }

    // Return std::nullopt for unexpected cases
    static constexpr std::nullopt_t unexpected(const std::optional<T>& opt) {
        return std::nullopt;
    }

    // Return std::nullopt for unexpected cases
    static constexpr std::nullopt_t unexpected(std::optional<T> && opt) {
        return std::nullopt;
    }
};

// Macro to simplify unwrapping
#define UNWRAP(expr) \
    ({ \
        const auto result = (expr); \
        using helper = ymd::__unwrap_helper<std::decay_t<decltype(result)>>; \
        if (unlikely(!helper::is_ok(result))) { \
            return helper::unexpected(result); \
        } \
        helper::unwrap(std::move(result)); \
    })

} // namespace ymd