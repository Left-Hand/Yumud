#include "j1939_primitive.hpp"

using namespace ymd;
using namespace ymd::j1939;
using namespace ymd::j1939::primitive;

namespace{
[[maybe_unused]] void test_pdn(){
    static constexpr auto pdn = Pdn::from_bits(0x12345678);

    static constexpr auto sa = pdn.sa().get();
    static_assert(sa.bits == 0x78);

    static constexpr auto ps = pdn.ps().get();
    static_assert(ps.bits == 0x56);

    static constexpr auto pf = pdn.pf().get();
    static_assert(pf.bits == 0x34);

    static constexpr auto dp = pdn.dp().get();
    static_assert(dp == DataPage::Presvered);

    static constexpr auto priority = pdn.priority().get();
    static_assert(static_cast<uint8_t>(priority) == 0x04);  
}
}