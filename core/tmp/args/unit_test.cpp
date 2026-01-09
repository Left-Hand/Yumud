#include "tuple_tmp.hpp"

using namespace ymd::tmp::tuple_tmp;

// 测试代码
static_assert(std::is_same_v<
    tuple_reduce_t<std::tuple<int, double, char, float>, std::is_integral>,
    std::tuple<double, float>
>);

static_assert(std::is_same_v<
    tuple_map_t<std::tuple<int, double, char>, std::add_const>,
    std::tuple<const int, const double, const char>
>);

static_assert(std::is_same_v<
    tuple_remove_specified_t<std::tuple<int, double, char, float>, double, char>,
    std::tuple<int, float>
>);

static_assert(std::is_same_v<
    tuple_replace_specified_t<std::tuple<int, double, char>, float, double, char>,
    std::tuple<int, float, float>
>);

static_assert(std::is_same_v<
    tuple_remove_specified_t<std::tuple<int, double>, float>,
    std::tuple<int, double>
>);

static_assert(std::is_same_v<
    tuple_replace_specified_t<std::tuple<>, float, int, double>,
    std::tuple<>
>);
