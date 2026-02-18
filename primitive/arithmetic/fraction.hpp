#pragma once

#include "core/stream/ostream.hpp"

namespace ymd{
template<typename T>
struct [[nodiscard]] Fraction {
    using Self = Fraction<T>;
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

    [[nodiscard]] static constexpr Self zero() {return Self{0, 1};}
    [[nodiscard]] static constexpr Self one() {return Self{1, 1};}
    [[nodiscard]] constexpr bool is_zero() const {return num == 0;} 
    [[nodiscard]] constexpr bool is_inf() const {return den == 0;}

    template<typename U>
    [[nodiscard]] constexpr bool is_similar_to(const Fraction<U> other) const {
        return num * other.den == den * other.num;
    }

    [[nodiscard]] constexpr bool operator==(const Fraction<T> other) const {
        return num == other.num && den == other.den;
    }

    template<typename U>
    [[nodiscard]] constexpr U to_floating() const {
        static_assert(std::is_arithmetic_v<U>);
        static_assert(std::is_integral_v<U> == false);

        return static_cast<U>(num) / den;
    }

    [[nodiscard]] constexpr explicit operator float() const {return to_floating<float>();} 

    friend OutputStream & operator << (OutputStream & os, const Self & self){
        return os << os.brackets<'{'>() << self.num << '/' << self.den << os.brackets<'}'>();
    }
};

}