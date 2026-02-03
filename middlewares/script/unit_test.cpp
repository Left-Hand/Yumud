#include "script_primitive.hpp"

using namespace ymd;
using namespace ymd::script;


[[maybe_unused]] static void static_test(){
    {
        static constexpr std::array ap = {StringView("114"), StringView("514")}; 

        static constexpr auto result = convert_params_to_tuple<std::tuple<int, int>>(ap);    
        static_assert(result.is_ok());
        static_assert(std::get<0>(result.unwrap()) == 114);
        static_assert(std::get<1>(result.unwrap()) == 514);
    }

    {
        static constexpr std::array ap = {StringView("1919")}; 

        static constexpr auto result = convert_params_to_tuple<std::tuple<int>>(ap);    
        static_assert(result.is_ok());
        static_assert(std::get<0>(result.unwrap()) == 1919);
    }
}
