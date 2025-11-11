#pragma once

#include <variant>
#include <type_traits>
#include <utility>

#include "Option.hpp"

#include "core/magic/args_traits.hpp"
#include "core/magic/enum_traits.hpp"

namespace ymd{

// Helper for overload pattern
template<class... Fns> struct overload : Fns... { 
    using Fns::operator()...; 
};
template<class... Fns> overload(Fns...) -> overload<Fns...>;

template<typename ... Ts>
class Sumtype{
private:
    template<typename T>
    constexpr explicit Sumtype(std::in_place_type_t<T>, T && value):
        value_(std::variant<Ts...>(
            std::in_place_type_t<T>(), std::forward<T>(value))){}
public:
    using Self = Sumtype<Ts...>;

    constexpr Sumtype() = delete;

    // Explicit construction from value (avoids implicit conversion issues)
    template<typename Raw >
    [[nodiscard]] static constexpr Sumtype from(Raw && value) {
        using T = std::decay_t<magic::first_convertible_arg_t<Raw, Ts...>>;
        return Sumtype(std::in_place_type<T>, std::forward<T>(value));
    }

    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr Sumtype(Raw && val):
        value_(std::in_place_type<T>, static_cast<T>(val)) { // Add std::in_place_type<Raw> to specify variant type
    }

    // 拷贝构造函数
    constexpr Sumtype(const Self& other) 
        noexcept((std::is_nothrow_copy_constructible_v<Ts> && ...))
        = default;
    
    // 移动构造函数
    constexpr Sumtype(Self&& other) 
        noexcept((std::is_nothrow_move_constructible_v<Ts> && ...))
        = default;
    
    // 拷贝赋值运算符
    constexpr Self& operator=(const Self& other) 
        noexcept((std::is_nothrow_copy_assignable_v<Ts> && ...)) 
        = default;
    
    // 移动赋值运算符
    constexpr Self& operator=(Self&& other) 
        noexcept((std::is_nothrow_move_assignable_v<Ts> && ...)) 
        = default;
    
    // 析构函数
    constexpr ~Sumtype() = default;



    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr bool is() const{
        return std::holds_alternative<T>(value_);
    }

    template<typename T>
    constexpr Option<T &> as() {
        if(! this->is<T>()) return None;
        return Some<T *>(&std::get<T>(value_)); 
    }

    template<typename T>
    constexpr const T & unwrap_as() const {
        if(! this->is<T>()) __builtin_abort();
        return std::get<T>(value_); 
    }

    template<typename T>
    constexpr Option<const T &> as() const {
        if(! this->is<T>()) return None;
        return Some<const T *>(&std::get<T>(value_)); 
    }


    // 相等比较
    constexpr bool operator==(const Self& other) const 
        noexcept((noexcept(std::declval<Ts>() == std::declval<Ts>()) && ...)) 
    {
        return value_ == other.value_;
    }

    template<typename Raw>
    constexpr bool operator==(const Raw& rhs) const noexcept {
        using T = magic::first_convertible_arg_t<Raw, Ts...>;
        static_assert(!std::is_same_v<T, void>, 
                      "No convertible type found for comparison");
        
        if (!is<T>()) return false;
        return unwrap_as<T>() == static_cast<T>(rhs);
    }

    template<typename Raw>
    constexpr bool operator!=(const Raw& rhs) const noexcept {
        return !(*this == rhs);
    }

    // Rust-style match functionality
    template<typename... Fns>
    constexpr decltype(auto) match(Fns&&... fns) const {
        return std::visit(overload(std::forward<Fns>(fns)...), value_);
    }

    template<typename Fn>
    constexpr decltype(auto) visit(Fn&& fn) const {
        return std::visit(std::forward<Fn>(fn), value_);
    }
    friend OutputStream & operator <<(OutputStream & os,const Sumtype & self){
        // 使用 std::visit 遍历 std::variant
        self.visit([&os](const auto& value) {
            os << value;
        });

        return os;
    }
private:
    std::variant<Ts...> value_;


    constexpr size_t var_index() const {
        return value_.index();
    }

};




}