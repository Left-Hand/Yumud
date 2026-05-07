#include "rm6623.hpp"


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
        constexpr auto code1 = RM6623CurrentCodeInterpreter::from_amps_bounded(1.0f);
        static_assert(abs_diff(RM6623CurrentCodeInterpreter::to_amps<float>(code1) ,1.0f) < 1e-3);

        constexpr auto code2 = RM6623CurrentCodeInterpreter::from_amps_bounded(5.0f);
        static_assert(abs_diff(RM6623CurrentCodeInterpreter::to_amps<float>(code2), 5.0f) < 1e-3);

        constexpr auto code3 = RM6623CurrentCodeInterpreter::from_amps_bounded(-5.0f);
        static_assert(abs_diff(RM6623CurrentCodeInterpreter::to_amps<float>(code3), -5.0f) < 1e-3);
    }
}

[[maybe_unused]] void test_calibrate_frame(){
    {
        static_assert(RM6623_CALIBRATE_CANFRAME.id_u32() == 0x3f0);
        static_assert(RM6623_CALIBRATE_CANFRAME.length() == 8);
        static_assert(RM6623_CALIBRATE_CANFRAME.payload()[0] == 'c');
        static_assert(RM6623_CALIBRATE_CANFRAME.payload()[1] == 0);
        static_assert(RM6623_CALIBRATE_CANFRAME.payload()[2] == 0);
        static_assert(RM6623_CALIBRATE_CANFRAME.payload()[3] == 0);
        static_assert(RM6623_CALIBRATE_CANFRAME.payload()[4] == 0);
        static_assert(RM6623_CALIBRATE_CANFRAME.payload()[5] == 0);
        static_assert(RM6623_CALIBRATE_CANFRAME.payload()[6] == 0);
        static_assert(RM6623_CALIBRATE_CANFRAME.payload()[7] == 0);
    }
}

}