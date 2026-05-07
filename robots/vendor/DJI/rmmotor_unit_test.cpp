#include "rmmotor_primitive.hpp"

using namespace ymd;
using namespace ymd::robots::dji;


namespace{


template<typename T>
static constexpr T abs_diff(const T a, const T b){
    if(b > a) return b - a;
    return a - b;
}
[[maybe_unused]] void test_codes(){


    {
        constexpr auto code1 = SpeedCode::from_tps(1.0f);
        static_assert(abs_diff((float)code1.to_tps<float>() ,1.0f) < 1e-3);

        constexpr auto code2 = SpeedCode::from_tps(20.0f);
        static_assert(abs_diff((float)code2.to_tps<float>(), 20.0f) < 1e-3);

        constexpr auto code3 = SpeedCode::from_tps(-20.0f);
        static_assert(abs_diff((float)code3.to_tps<float>(), -20.0f) < 1.3e-3);
    }
    
    {
        constexpr auto code1 = SpeedCode::from_tps(1.0f);
        static_assert(abs_diff((float)code1.to_tps<iq13>() ,1.0f) < 1e-3);

        constexpr auto code2 = SpeedCode::from_tps(20.0f);
        static_assert(abs_diff((float)code2.to_tps<iq13>(), 20.0f) < 1e-3);

        constexpr auto code3 = SpeedCode::from_tps(-20.0f);
        static_assert(abs_diff((float)code3.to_tps<iq13>(), -20.0f) < 1.3e-3);
    }
}

[[maybe_unused]] void test_ser_frame(){

}


}