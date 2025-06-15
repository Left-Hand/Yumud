#pragma once

#include "core/magic/function_traits.hpp"

namespace ymd{
template<typename ... Ts>
class SumtypeEnum{
public:
    using Self = SumtypeEnum<Ts...>;
    // using Ts...;

    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr SumtypeEnum(Raw && val):
        value_(std::in_place_type<T>, static_cast<T>(val)) {
    }

    
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
    constexpr Option<const T &> as() const {
        if(! this->is<T>()) return None;
        return Some<const T *>(&std::get<T>(value_)); 
    }


    constexpr bool operator ==(const Self & other) const {
        return this->value_ == other.value_;
    }

    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr bool operator ==(const Raw &rhs) const {
        if(not this->is<T>()) return false;
        return this->as<T>().unwrap() == static_cast<T>(rhs);
    }

    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr bool operator !=(const Raw &rhs) const {
        return !(this->operator ==(rhs));
    }

    template<typename Fn, typename Ret = magic::functor_ret_t<Fn>>
    Ret visit(Fn && fn) const {
        auto & self = *this;
        return std::visit([&](const auto & value) {
            // using T = std::decay_t<decltype(value)>;

            std::forward<Fn>(fn)(value);
        }, self.value_);
    }

    friend OutputStream & operator <<(OutputStream & os,const SumtypeEnum & self){
        // 使用 std::visit 遍历 std::variant
        std::visit([&os](const auto& value) {
            using T = std::decay_t<decltype(value)>;

            // 检查类型是否可被 OutputStream 打印
            if constexpr (requires(OutputStream& os, const T& value) {os << value;}) {
                os << value; // 如果可打印，则直接打印
            } else {
                os << "[Unprintable type]"; // 否则打印提示信息
            }
        }, self.value_);

        return os;
    }
private:
    std::variant<Ts...> value_;

    constexpr size_t var_index() const {
        return value_.index();
    }
};

}