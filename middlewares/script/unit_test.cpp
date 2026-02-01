#include "script_primitive.hpp"

using namespace ymd::script;

#if 0

[[maybe_unused]] static void static_test(){
    {
        constexpr std::array ap = {"114", "514"}; 

        constexpr auto result = convert_params_to_tuple<std::tuple<int, int>>(
            ap, std::make_index_sequence<2>{});    
        static_assert(result.is_ok());
        static_assert(std::get<0>(result.unwrap()) == 114);
        static_assert(std::get<1>(result.unwrap()) == 514);
    }

    {
        constexpr std::array ap = {"1919"}; 

        constexpr auto result = convert_params_to_tuple<std::tuple<int>>(
            ap, std::make_index_sequence<1>{});    
        static_assert(result.is_ok());
        static_assert(std::get<0>(result.unwrap()) == 1919);
    }
}

#endif
