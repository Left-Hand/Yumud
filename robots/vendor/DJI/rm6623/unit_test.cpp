#include "rm6623.hpp"


using namespace ymd;
using namespace ymd::robots::dji::rm6623;


namespace{


template<typename T>
static constexpr T abs_diff(const T a, const T b){
    if(b > a) return b - a;
    return a - b;
}
[[maybe_unused]] void test_codes(){
    {
        constexpr auto code1 = CurrentCode::from_amps_bounded(1.0f);
        static_assert(abs_diff(code1.to_amps<float>() ,1.0f) < 1e-3);

        constexpr auto code2 = CurrentCode::from_amps_bounded(3.0f);
        static_assert(abs_diff(code2.to_amps<float>(), 3.0f) < 1e-3);

        constexpr auto code3 = CurrentCode::from_amps_bounded(-3.0f);
        static_assert(abs_diff(code3.to_amps<float>(), -3.0f) < 1e-3);
    }
}

}