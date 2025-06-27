#pragma once

#include <tuple>
#include <variant>
#include <cstdint>



namespace ymd::magic{
namespace details{
template<typename Tup>
struct tuple_to_variant{
};

template<typename... Ts>
struct tuple_to_variant<std::tuple<Ts...>> {
    using type = std::variant<Ts...>;
};

template<typename Var>
struct variant_to_tuple{
};

template<typename... Ts>
struct variant_to_tuple<std::variant<Ts...>> {
    using type = std::tuple<Ts...>;
};

template<size_t I, typename Variant>
struct variant_get_trait{
    static constexpr auto get(const Variant & var, size_t index){
        return std::get<index>(var);
    }
};



template <typename T, typename... Ts>
struct tuple_erase_duplicate : std::type_identity<T> {};

template <typename... Ts, typename U, typename... Us>
struct tuple_erase_duplicate<std::tuple<Ts...>, U, Us...>
    : std::conditional_t<(std::is_same_v<U, Ts> || ...)
    , tuple_erase_duplicate<std::tuple<Ts...>, Us...>
    , tuple_erase_duplicate<std::tuple<Ts..., U>, Us...>> {};


// is_instantiation_of
template <typename Inst, template <typename...> typename Tmpl>
struct is_instantiation_of : std::false_type {};

template <template <typename...> typename Tmpl, typename... Args>
struct is_instantiation_of<Tmpl<Args...>, Tmpl> : std::true_type {};
}

template<typename Tup>
using tuple_to_variant_t = typename details::tuple_to_variant<Tup>::type;

template<typename Var>
using variant_to_tuple_t = typename details::variant_to_tuple<Var>::type;

template<size_t I, typename Variant>
using variant_element_t = std::tuple_element_t<I, variant_to_tuple_t<Variant>>;

static_assert(std::is_same_v<
    tuple_to_variant_t<std::tuple<int, float, bool>>,
    std::variant<int, float, bool>
>);


template <typename... Ts>
using tuple_erase_duplicate_t = typename details::
    tuple_erase_duplicate<std::tuple<>, Ts...>::type;
}

namespace ymd{


template<typename T>
using config_to_task_t = typename T::Task;

template<typename ConfigsTuple, typename IndexSeq>
struct configs_tuple_to_tasks_variant_impl;

template<typename... Configs, std::size_t... Is>
struct configs_tuple_to_tasks_variant_impl<
    std::tuple<Configs...>, std::index_sequence<Is...>>
{
    using type = magic::tuple_to_variant_t<
        magic::tuple_erase_duplicate_t<
            typename std::tuple_element_t<Is, std::tuple<Configs...>>::Task...>>;
};

template<typename ConfigsTuple>
using configs_tuple_to_tasks_variant_t = 
    typename configs_tuple_to_tasks_variant_impl<ConfigsTuple,
    std::make_index_sequence<std::tuple_size_v<ConfigsTuple>>>::type;

template<typename ConfigsTuple, typename IndexSeq>
struct configs_tuple_to_dignosis_variant_impl;

template<typename... Configs, std::size_t... Is>
struct configs_tuple_to_dignosis_variant_impl<
    std::tuple<Configs...>, std::index_sequence<Is...>> {
    using type = magic::tuple_to_variant_t<
        magic::tuple_erase_duplicate_t<
            typename std::tuple_element_t<Is, 
                std::tuple<Configs...>>::Task::Dignosis...>>;
};

template<size_t I, typename ConfigsTuple>
using idx_to_task_t = typename std::tuple_element_t<I, ConfigsTuple>::Task;

template<typename ConfigsTuple>
using configs_tuple_to_dignosis_variant_t = typename 
    configs_tuple_to_dignosis_variant_impl<ConfigsTuple,
    std::make_index_sequence<std::tuple_size_v<ConfigsTuple>>
>::type;


template<typename T, typename R>
__fast_inline constexpr T map_nearest(const T value, R && range){
    auto it = std::begin(range);
    auto end = std::end(range);
    
    T nearest = *it;
    auto min_diff = ABS(value - nearest);
    
    while(++it != end) {
        const auto current = *it;
        const auto diff = ABS(value - current);
        if(diff < min_diff) {
            min_diff = diff;
            nearest = current;
        }
    }
    return nearest;
}

static constexpr void static_test(){
    static_assert(map_nearest(0, std::initializer_list<int>{1,-3,-5}) == 1);
}

}