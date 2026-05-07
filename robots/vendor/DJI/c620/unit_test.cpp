#include "c620.hpp"

using namespace ymd;
using namespace ymd::robots::dji;
using namespace ymd::robots::dji::c620;


namespace{


template<typename T>
static constexpr T abs_diff(const T a, const T b){
    if(b > a) return b - a;
    return a - b;
}
[[maybe_unused]] void test_codes(){
    {
        constexpr auto code1 = C620CurrentCodeInterpreter::from_amps_bounded(1.0f);
        static_assert(abs_diff(C620CurrentCodeInterpreter::to_amps<float>(code1) ,1.0f) < 1e-3);

        constexpr auto code2 = C620CurrentCodeInterpreter::from_amps_bounded(20.0f);
        static_assert(abs_diff(C620CurrentCodeInterpreter::to_amps<float>(code2), 20.0f) < 1e-3);

        constexpr auto code3 = C620CurrentCodeInterpreter::from_amps_bounded(-20.0f);
        static_assert(abs_diff(C620CurrentCodeInterpreter::to_amps<float>(code3), -20.0f) < 1e-3);
    }

    {
        constexpr auto code1 = C610CurrentCodeInterpreter::from_amps_bounded(1.0f);
        static_assert(abs_diff(C610CurrentCodeInterpreter::to_amps<float>(code1) ,1.0f) < 1e-3);

        constexpr auto code2 = C610CurrentCodeInterpreter::from_amps_bounded(10.0f);
        static_assert(abs_diff(C610CurrentCodeInterpreter::to_amps<float>(code2), 10.0f) < 1e-3);

        constexpr auto code3 = C610CurrentCodeInterpreter::from_amps_bounded(-10.0f);
        static_assert(abs_diff(C610CurrentCodeInterpreter::to_amps<float>(code3), -10.0f) < 1e-3);
    }

    {
        constexpr auto code1 = C620CurrentCodeInterpreter::from_amps_bounded(1.0_iq13);
        static_assert(abs_diff((float)C620CurrentCodeInterpreter::to_amps<iq13>(code1) ,1.0f) < 1e-3);

        constexpr auto code2 = C620CurrentCodeInterpreter::from_amps_bounded(20.0_iq13);
        static_assert(abs_diff((float)C620CurrentCodeInterpreter::to_amps<iq13>(code2), 20.0f) < 1e-3);

        constexpr auto code3 = C620CurrentCodeInterpreter::from_amps_bounded(-20.0_iq13);
        static_assert(abs_diff((float)C620CurrentCodeInterpreter::to_amps<iq13>(code3), -20.0f) < 1.3e-3);
    }

    {
        constexpr auto code1 = C620CurrentCodeInterpreter::from_amps_bounded(1.0_iq16);
        static_assert(abs_diff((float)C620CurrentCodeInterpreter::to_amps<iq16>(code1) ,1.0f) < 1e-3);

        constexpr auto code2 = C620CurrentCodeInterpreter::from_amps_bounded(20.0_iq16);
        static_assert(abs_diff((float)C620CurrentCodeInterpreter::to_amps<iq16>(code2), 20.0f) < 1e-3);

        constexpr auto code3 = C620CurrentCodeInterpreter::from_amps_bounded(-20.0_iq16);
        static_assert(abs_diff((float)C620CurrentCodeInterpreter::to_amps<iq16>(code3), -20.0f) < 1.3e-3);
    }





}


}