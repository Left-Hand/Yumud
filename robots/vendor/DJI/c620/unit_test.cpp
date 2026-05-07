#include "c620.hpp"

using namespace ymd;
using namespace ymd::robots::dji::c620;


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

        constexpr auto code2 = CurrentCode::from_amps_bounded(20.0f);
        static_assert(abs_diff(code2.to_amps<float>(), 20.0f) < 1e-3);

        constexpr auto code3 = CurrentCode::from_amps_bounded(-20.0f);
        static_assert(abs_diff(code3.to_amps<float>(), -20.0f) < 1e-3);
    }

    {
        constexpr auto code1 = CurrentCode::from_amps_bounded_c610(1.0f);
        static_assert(abs_diff(code1.to_amps_c610<float>() ,1.0f) < 1e-3);

        constexpr auto code2 = CurrentCode::from_amps_bounded_c610(10.0f);
        static_assert(abs_diff(code2.to_amps_c610<float>(), 10.0f) < 1e-3);

        constexpr auto code3 = CurrentCode::from_amps_bounded_c610(-10.0f);
        static_assert(abs_diff(code3.to_amps_c610<float>(), -10.0f) < 1e-3);
    }

    {
        constexpr auto code1 = CurrentCode::from_amps_bounded(1.0_iq16);
        static_assert(abs_diff((float)code1.to_amps<iq16>() ,1.0f) < 1e-3);

        constexpr auto code2 = CurrentCode::from_amps_bounded(20.0_iq16);
        static_assert(abs_diff((float)code2.to_amps<iq16>(), 20.0f) < 1e-3);

        constexpr auto code3 = CurrentCode::from_amps_bounded(-20.0_iq16);
        static_assert(abs_diff((float)code3.to_amps<iq16>(), -20.0f) < 1.3e-3);
    }


    {
        constexpr auto code1 = CurrentCode::from_amps_bounded(1.0_iq13);
        static_assert(abs_diff((float)code1.to_amps<iq13>() ,1.0f) < 1e-3);

        constexpr auto code2 = CurrentCode::from_amps_bounded(20.0_iq13);
        static_assert(abs_diff((float)code2.to_amps<iq13>(), 20.0f) < 1e-3);

        constexpr auto code3 = CurrentCode::from_amps_bounded(-20.0_iq13);
        static_assert(abs_diff((float)code3.to_amps<iq13>(), -20.0f) < 1.3e-3);
    }

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