#pragma once

#include "magic_details.hpp"

// https://taylorconor.com/blog/enum-reflection/

namespace ymd::magic{

template <typename E, E V> 
constexpr std::string_view enum_item_name_v = magic::details::_enum_item_name<E, V>();

template <typename E, E V> 
constexpr bool enum_is_valid_v = magic::details::_enum_is_valid<E, V>();

template <typename E>
constexpr size_t enum_count_v = magic::details::_enum_internal_element_count<E>(
    std::make_integer_sequence<size_t, 100>());

namespace details{
template<
    typename E, 
    size_t I,
    typename Int = std::underlying_type_t<E>,
    Int E_RAW
>
static consteval E _enum_find_item(){
    constexpr Int ENUM_INT_MIN = std::numeric_limits<Int>::min();
    constexpr Int ENUM_INT_MAX = std::numeric_limits<Int>::max();
    static_assert(ENUM_INT_MAX - ENUM_INT_MIN <= 256);
    constexpr bool is_valid = magic::enum_is_valid_v<E, static_cast<E>(E_RAW)>;
    if constexpr(is_valid){
        if constexpr (I == 0) return static_cast<E>(E_RAW);
        else return _enum_find_item<E, I - 1, Int, E_RAW + 1>();
    }else{
        return _enum_find_item<E, I, Int, E_RAW + 1>();
    }
}



template<typename E, size_t I>
static constexpr E _enum_item_v(){
    using Int = std::underlying_type_t<E>;
    constexpr Int ENUM_INT_MIN = std::numeric_limits<Int>::min();
    return _enum_find_item<E, I, Int, ENUM_INT_MIN>();
}
}

template<typename E, size_t I>
static constexpr E enum_item_v = details::_enum_item_v<E, I>();
template<typename Enum>
struct enum_dump{
    static constexpr size_t COUNT = magic::enum_count_v<Enum>;
    using Int = std::underlying_type_t<Enum>;

    using IndexStorage = std::array<Int, COUNT>;
    using StringsStorage = std::array<std::string_view, COUNT>;

    static constexpr std::optional<std::string_view> enum_to_str(const Enum value){
        const auto index = enum_to_rank(value);
        if(index.has_value()) return strings_storage[index.value()];
        else return std::nullopt;
    }

    static constexpr std::optional<Enum> str_to_enum(const std::string_view str){
        for(size_t i = 0; i < COUNT; ++i){
            if(str == strings_storage[i]) return static_cast<Enum>(index_storage[i]);
        }
        return std::nullopt;
    }

    static constexpr std::optional<size_t> enum_to_rank(const Enum value){
        auto it = std::lower_bound(index_storage.begin(), index_storage.end(), static_cast<Int>(value));
        if (it != index_storage.end() && *it == static_cast<Int>(value)) {
            return static_cast<int>(it - index_storage.begin());
        }
        return std::nullopt;
    }
private:


    template<size_t... I>
    static constexpr auto make_index_storage(std::index_sequence<I...>) {
        return IndexStorage{static_cast<Int>(magic::enum_item_v<Enum, I>)...};
    };
    
    
    template<size_t... I>
    static constexpr auto make_strings_storage(std::index_sequence<I...>) {
        return StringsStorage{magic::enum_item_name_v<Enum, magic::enum_item_v<Enum, I>>...};
    };

    static constexpr IndexStorage index_storage = make_index_storage(std::make_index_sequence<COUNT>{});
    static constexpr StringsStorage strings_storage = make_strings_storage(std::make_index_sequence<COUNT>{});
};
}

namespace ymd{
template<typename T>
struct DeriveDebugDispatcher;

template<typename E>
requires std::is_enum_v<E>
struct DeriveDebugDispatcher<E> {
    static void call(OutputStream& os, const E value) {
        const auto str_opt = magic::enum_dump<E>::enum_to_str(value);
        if (str_opt.has_value()) {
            os << str_opt.value();
        } else {
            os << static_cast<std::underlying_type_t<E>>(value);
        }
    }
};
}
