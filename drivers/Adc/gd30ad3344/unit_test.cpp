#include "gd30ad3344.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Self = GD30AD3344;


namespace{

[[maybe_unused]] static void test_conf_code(){

    {
        constexpr auto code = Self::ConfCode::from_default();
        static_assert(code.to_u16() == 0x058b);
        static_assert((uint16_t)code.mux == 0);
    }
}
}