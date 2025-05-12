#pragma once

#include "typetraits_details.hpp"

namespace ymd::magic{




template <typename E, E V> 
constexpr const char * enum_item_name_v = magic::details::_enum_item_name<E, V>();

template <typename E, E V> 
constexpr bool enum_is_valid_v = magic::details::_enum_is_valid<E, V>();



template <typename E>
constexpr int enum_count_v = magic::details::_enum_internal_element_count<E>(
    std::make_integer_sequence<int, 100>());


}