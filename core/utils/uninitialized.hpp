#pragma once

#include <type_traits>

namespace ymd{

template<typename T>
struct FromUninitializedDispatcher {
    
    // 优先级3：静态成员函数 T::from_uninitialized()
    static consteval T from_uninitialized() 
        requires requires { 
            { T::from_uninitialized() } -> std::same_as<T>;
        }
    {
        return T::from_uninitialized();
    }
    
    
    // 如果没有合适的方法，删除函数
    static consteval T from_uninitialized() = delete;
};


struct _Uninitialized{
    template<typename T>
    consteval operator T() const {
        return FromUninitializedDispatcher<T>::from_uninitialized();
    }
};

static constexpr inline _Uninitialized Uninitialized = _Uninitialized{};
}