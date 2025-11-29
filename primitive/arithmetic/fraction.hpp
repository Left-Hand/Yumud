#pragma once

#include <type_traits>


namespace ymd{
template<typename T>
struct [[nodiscard]] Fraction {
    using num_t = decltype([] {
        if constexpr (std::is_arithmetic_v<T>) {
            return T{};
        } else if constexpr (requires { typename T::num_t; }) {
            return typename T::num_t{};
        } else {
            return T{};
        }
    }());
    
    using den_t = decltype([] {
        if constexpr (std::is_arithmetic_v<T>) {
            return T{};
        } else if constexpr (requires { typename T::den_t; }) {
            return typename T::den_t{};
        } else {
            return T{};
        }
    }());



    num_t num;
    den_t den;
};

}