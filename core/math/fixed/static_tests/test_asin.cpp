#include "setup_test.hpp"
#include "../fxmath/asin_acos.hpp"

namespace {

[[maybe_unused]] static void test_asin(){
    static_assert(std::abs((double)math::asin(iq16(0)) - (0)) < 1E-7);
    static_assert(std::abs((double)math::asin(iq16(1.0)) - (M_PI / 2)) < 1E-7);
    static_assert(std::abs((double)math::asin(iq16(-1.0)) - (-M_PI / 2)) < 1E-7);

    constexpr auto e1 = std::abs((double)math::asin(iq16(0.707106781187)) - (M_PI / 4));
    static_assert(e1 < 2.06E-4);

    static_assert(std::abs((double)math::asin(iq16(0.866025403784)) - (M_PI / 3)) < 1E-4);
    
    static_assert(std::abs((double)math::asin(iq16(0.5)) - (M_PI / 6)) < 1E-7);

    constexpr auto e2 = std::abs((double)math::asin(iq16(0.7)) - (0.775397496611));
    static_assert(e2 < 1E-4);
}

}