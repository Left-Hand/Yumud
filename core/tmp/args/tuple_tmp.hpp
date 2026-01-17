#include <tuple>


namespace ymd::tmp::tuple_tmp::details {

// 基础工具：判断类型是否在包中
template<typename T, typename... Args>
struct type_in_pack : std::false_type {};

template<typename T, typename First, typename... Rest>
struct type_in_pack<T, First, Rest...> 
    : std::conditional_t<std::is_same_v<T, First>, 
                        std::true_type, 
                        type_in_pack<T, Rest...>> {};

template<typename T>
struct type_in_pack<T> : std::false_type {};

// 元组移除指定类型的实现
template<typename Tuple, typename... RemoveTypes>
struct tuple_remove_specified_impl;

// 递归基案：空元组
template<typename... RemoveTypes>
struct tuple_remove_specified_impl<std::tuple<>, RemoveTypes...> {
    using type = std::tuple<>;
};

// 递归步骤
template<typename First, typename... Rest, typename... RemoveTypes>
struct tuple_remove_specified_impl<std::tuple<First, Rest...>, RemoveTypes...> {
private:
    using remaining_tuple = typename tuple_remove_specified_impl<
        std::tuple<Rest...>, RemoveTypes...>::type;
    
public:
    using type = std::conditional_t<
        type_in_pack<First, RemoveTypes...>::value,
        remaining_tuple,
        decltype(std::tuple_cat(std::declval<std::tuple<First>>(), 
            std::declval<remaining_tuple>()))
    >;
};

// 元组替换指定类型的实现
template<typename Tuple, typename NewType, typename... ReplaceTypes>
struct tuple_replace_specified_impl;

// 递归基案：空元组
template<typename NewType, typename... ReplaceTypes>
struct tuple_replace_specified_impl<std::tuple<>, NewType, ReplaceTypes...> {
    using type = std::tuple<>;
};

// 递归步骤
template<typename First, typename... Rest, typename NewType, typename... ReplaceTypes>
struct tuple_replace_specified_impl<std::tuple<First, Rest...>, NewType, ReplaceTypes...> {
private:
    using remaining_tuple = typename tuple_replace_specified_impl<
        std::tuple<Rest...>, NewType, ReplaceTypes...>::type;
    
    using current_type = std::conditional_t<
        type_in_pack<First, ReplaceTypes...>::value,
        NewType,
        First
    >;
    
public:
    using type = decltype(std::tuple_cat(
        std::declval<std::tuple<current_type>>(),
        std::declval<remaining_tuple>()
    ));
};

// 元组映射的实现
template<template<typename> class Mapper, typename Tuple>
struct tuple_map_impl;

// 递归基案：空元组
template<template<typename> class Mapper>
struct tuple_map_impl<Mapper, std::tuple<>> {
    using type = std::tuple<>;
};

// 递归步骤
template<template<typename> class Mapper, typename First, typename... Rest>
struct tuple_map_impl<Mapper, std::tuple<First, Rest...>> {
private:
    using mapped_first = typename Mapper<First>::type;
    using mapped_rest = typename tuple_map_impl<Mapper, std::tuple<Rest...>>::type;
    
public:
    using type = decltype(std::tuple_cat(
        std::declval<std::tuple<mapped_first>>(),
        std::declval<mapped_rest>()
    ));
};

// 元组规约的实现 - 根据策略剔除元素
template<template<typename> class Predicate, typename Tuple>
struct tuple_reduce_impl;

// 递归基案：空元组
template<template<typename> class Predicate>
struct tuple_reduce_impl<Predicate, std::tuple<>> {
    using type = std::tuple<>;
};

// 递归步骤
template<template<typename> class Predicate, typename First, typename... Rest>
struct tuple_reduce_impl<Predicate, std::tuple<First, Rest...>> {
private:
    using remaining_tuple = typename tuple_reduce_impl<Predicate, std::tuple<Rest...>>::type;
    
public:
    using type = std::conditional_t<
        Predicate<First>::value,
        remaining_tuple,
        decltype(std::tuple_cat(std::declval<std::tuple<First>>(), 
            std::declval<remaining_tuple>()))
    >;
};

// 支持模板别名版本的谓词（如 std::is_integral_v 的适配器）
template<template<typename> class Predicate, typename Tuple>
struct tuple_reduce_alias_impl;

template<template<typename> class Predicate>
struct tuple_reduce_alias_impl<Predicate, std::tuple<>> {
    using type = std::tuple<>;
};

template<template<typename> class Predicate, typename First, typename... Rest>
struct tuple_reduce_alias_impl<Predicate, std::tuple<First, Rest...>> {
private:
    using remaining_tuple = typename tuple_reduce_alias_impl<Predicate, std::tuple<Rest...>>::type;
    
public:
    using type = std::conditional_t<
        Predicate<First>::value,
        remaining_tuple,
        decltype(std::tuple_cat(std::declval<std::tuple<First>>(), 
        std::declval<remaining_tuple>()))
    >;
};

} // namespace details

namespace ymd::tmp::tuple_tmp{
// 主模板别名
template<typename T, typename... Args>
using tuple_remove_specified_t = typename details::tuple_remove_specified_impl<T, Args...>::type;

template<typename T, typename U, typename... Args>
using tuple_replace_specified_t = typename details::tuple_replace_specified_impl<T, U, Args...>::type;

// 主模板别名
template<typename Tuple, template<typename> class Mapper>
using tuple_map_t = typename details::tuple_map_impl<Mapper, Tuple>::type;


// 主模板别名 - 自动检测谓词类型
template<typename Tuple, template<typename> class Predicate>
using tuple_reduce_t = typename details::tuple_reduce_alias_impl<Predicate, Tuple>::type;

}