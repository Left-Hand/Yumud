#include "crsf_ser.hpp"

using namespace ymd;
using namespace ymd::crsf;

namespace {


[[maybe_unused]] static void test_ser_funcs(){
    {
        [[maybe_unused]] static constexpr std::array<uint8_t, 8> obj{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        // static constexpr auto [bytes, res] = []{
        //     std::array<uint8_t, 8> _bytes;
        //     const auto res = SerialzeFunctions::ser_0xff_terminated_uchars(_bytes, obj);
        //     return std::make_tuple(_bytes, res);
        // }();
    }
}
}