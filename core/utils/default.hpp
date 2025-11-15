#pragma once

namespace ymd{

template<typename T>
struct FromDefaultDispatcher {
    // 优先级1：静态常量成员 T::DEFAULT
    static consteval T from_default() 
        requires requires { 
            T::DEFAULT; 
            requires std::same_as<decltype(T::DEFAULT), const T>;
        }
    {
        return T::DEFAULT;
    }
    
    // 优先级2：枚举的 Default 项
    static consteval T from_default() 
        requires requires { 
            requires std::is_enum_v<T>;
            T::Default;
            requires std::same_as<decltype(T::Default), const T>;
        }
    {
        return T::Default;
    }
    
    // 优先级3：静态成员函数 T::from_default()
    static consteval T from_default() 
        requires requires { 
            { T::from_default() } -> std::same_as<T>;
        }
    {
        return T::from_default();
    }
    
    
    // 如果没有合适的方法，删除函数
    static consteval T from_default() = delete;
};


struct _Default{
    template<typename T>
    consteval operator T() const {
        return FromDefaultDispatcher<T>::from_default();
    }
};

static constexpr inline _Default Default = _Default{};
}