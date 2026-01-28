#pragma once

#include <type_traits>

namespace ymd::tmp{
// 辅助类型特征检测
template <template<typename...> class, typename...>
struct is_instantiation_of : std::false_type {};

template <template<typename...> class U, typename... Ts>
struct is_instantiation_of<U, U<Ts...>> : std::true_type {};

template <typename T, template<typename...> class U>
using is_instantiation_of_t = typename is_instantiation_of<U, T>::type;
}