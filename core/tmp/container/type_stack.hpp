#pragma once

#include "tuple"

namespace yumud::tmp{

template<typename First, typename ... Ts, typename Last>
struct stack{
    using tuple_type = std::tuple<First, Ts..., Last>;

    template<typename T>
    using push_back_type = stack<First, Ts..., Last, T>;

    using pop_back_type = stack<First, Ts...>;

    template<typename T>
    using push_front_type = stack<T, First, Ts..., Last>;

    using pop_front_type = stack<Ts..., Last>;

    // 修复 front_type 和 back_type
    using front_type = First;
    using back_type = Last;
    
    // 如果需要通过索引访问，可以添加这些辅助类型
    static constexpr size_t size = sizeof...(Ts) + 2;
    
    template<size_t I>
    using element_type = std::tuple_element_t<I, tuple_type>;
};
}