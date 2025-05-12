#pragma once

#include "../tb.h"

#include "core/math/real.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/clock/clock.hpp"

#include "core/utils/typetraits/size_traits.hpp"
#include "core/utils/typetraits/function_traits.hpp"
// #include "core/utils/typetraits/typetraits_details.hpp"
#include "core/utils/typetraits/serialize_traits.hpp"
// https://taylorconor.com/blog/enum-reflection/




// template<typename T>
// struct function_traits;


// template<typename Functor, typename Ret, typename... Args>
// struct function_traits<Ret(Functor::*)(Args...) > {
//     using return_type = Ret;
//     using args_type = std::tuple<Args...>;
// };

// template<typename Functor, typename... Args>
// struct function_traits<void(Functor::*)(Args...)> {
//     using return_type = void;
//     using args_type = std::tuple<Args...>;
// };

// template<typename Functor, typename Ret>
// struct function_traits<Ret(Functor::*)(void)> {
//     using return_type = Ret;
//     using args_type = std::tuple<void>;
// };

// template<typename Functor>
// struct function_traits<void(Functor::*)(void)> {
//     using return_type = void;
//     using args_type = std::tuple<void>;
// };



namespace ymd{





template <typename E, E V> 
constexpr const char * enum_item_name_v = magic::details::_enum_item_name<E, V>();

template <typename E, E V> 
constexpr bool enum_is_valid_v = magic::details::_enum_is_valid<E, V>();



template <typename E>
constexpr int enum_count_v = magic::details::_enum_internal_element_count<E>(
    std::make_integer_sequence<int, 100>());





enum Fruit {
    BANANA = 5,
    APPLE = 12,
};



// // template<std::span<const uint8_t> ... Ts>
// void apply_serval_bytes(const int ... pieces) {
//     auto handler = [](const auto& piece) {  // 使用auto兼容不同参数类型
//         DEBUG_PRINTLN(piece);
//     };
//     (handler(pieces), ...);  // 折叠表达式展开调用
// }

// C++20 修复方案（假设参数类型为 std::span<const uint8_t>）

template<typename ... Ts>
void apply_serval_bytes(Ts && ... pieces) {
    (DEBUG_PRINTLN(pieces), ...);  // 直接展开参数包
}

}