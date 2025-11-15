#pragma once

#include <concepts>

namespace ymd{

//默认实现 使用T类型的ZERO常量或from_zero方法 实现非入侵的零值获取
template<typename T>
struct FromZeroDispatcher {
    // 优先：静态常量成员 T::ZERO
    static consteval T from_zero() 
        requires requires {
            T::ZERO;
            requires std::same_as<decltype(T::ZERO), const T>;
            requires std::is_constant_evaluated(); // 确保在编译期
        }
    {
        return T::ZERO;
    }
    
    // 备选：静态成员函数 T::from_zero()
    static consteval T from_zero() 
        requires requires {
            { T::from_zero() } -> std::same_as<T>;
        }
    {
        return T::from_zero();
    }
};

template<typename T>
requires std::is_arithmetic_v<T>
struct FromZeroDispatcher<T>{
    static consteval T from_zero() {
        return static_cast<T>(0);
    }
}; 

struct _Zero{
    template<typename T>
    consteval operator T() const {
        return FromZeroDispatcher<T>::from_zero();
    }
};

static constexpr inline _Zero Zero = _Zero{};


}