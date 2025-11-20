#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>


namespace ymd::tmp{

// 只有在特化时才能够确定值 可用于constexpr排除分支下的static_assert
template <typename T>
static constexpr bool false_v = false;

template <typename T>
static constexpr bool true_v = true;


}