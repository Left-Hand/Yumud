#include "setup_test.hpp"
#include "../fxmath/atan2.hpp"

namespace {
[[maybe_unused]] static void test_atan(){
    static_assert(std::abs((double)math::atan2pu(iq16(1.0), iq16(1.0)) - 0.125) < 1E-7);
    static_assert(std::abs((double)math::atan2pu(-iq16(1.0), -iq16(1.0)) - 0.625) < 1E-7);
    static_assert(std::abs((double)math::atan2pu(iq16(1.0), -iq16(1.0)) - 0.375) < 1E-7);
    static_assert(std::abs((double)math::atan2pu(-iq16(1.0), iq16(1.0)) - 0.875) < 1E-7);

    static_assert(std::abs((double)math::atanpu(iq16(1.0)) - 0.125) < 1E-7);
    static_assert(std::abs((double)math::atanpu(-iq16(1.0)) - 0.875) < 1E-7);

    static constexpr double SQRT3_F64 = std::sqrt(3.0);
    
    static_assert(err_f64(math::atanpu(iq29(1.0 / SQRT3_F64)), uq32(0.25 * (1.0 / 3))) < 1E-7);
    static_assert(err_f64(math::atanpu(iq29(SQRT3_F64)), uq32(0.25 * (2.0 / 3))) < 1E-7);

    static_assert(err_f64(math::atanpu(iq29(-1.0 / SQRT3_F64)), uq32(1.0 - 0.25 * (1.0 / 3))) < 1E-7);
    static_assert(err_f64(math::atanpu(iq29(-SQRT3_F64)), uq32(1.0 - 0.25 * (2.0 / 3))) < 1E-7);
}

}