#include "vesc_can_msgs.hpp"
#include "vesc_can_serialize.hpp"


using namespace ymd;
using namespace ymd::robots::vesc::can;

namespace{

[[maybe_unused]] void test_ser(){

    {
        constexpr auto msg = GnssTime{
            .ms_today = 0x1,
            .yy = 0x2,
            .mo = 0x3,
            .dd = 0x4,
        };

        constexpr auto can_frame = make_can_frame(5, msg);

        static_assert(can_frame.id_u32() == (59 * 256 + 5));
        static_assert(can_frame.length() == 8);
        static_assert(can_frame.payload()[0] == 0x01);

        static_assert(can_frame.payload()[4] == 0x02);
        static_assert(can_frame.payload()[6] == 0x03);
        static_assert(can_frame.payload()[7] == 0x04);

    }
}
}