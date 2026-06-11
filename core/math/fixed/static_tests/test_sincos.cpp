#include "setup_test.hpp"
#include "../fxmath/sin_cos.hpp"

namespace {

[[maybe_unused]] static void test_sin(){
    static_assert(math::sinpu(iq16(0)).to_bits() == 0);
    static_assert(math::sinpu(iq16(0.25)).to_bits() == std::numeric_limits<iq31>::max().to_bits());
    static_assert(math::sinpu(iq16(0.5)).to_bits() == iq16(0).to_bits());
    static_assert(math::sinpu(iq16(1.0)).to_bits() == iq16(0).to_bits());

    static_assert(err_f64(math::sinpu(iq31(0.25 *(1.0 / 3))), iq31(0.5)) < 1E-7);
    static_assert(err_f64(math::sinpu(iq31(0.25 *(5.0 / 3))), iq31(0.5)) < 1E-7);
    static_assert(err_f64(math::sinpu(iq31(0.25 *(7.0 / 3))), iq31(-0.5)) < 1E-7);
    static_assert(err_f64(math::sinpu(iq31(0.25 *(11.0 / 3))), iq31(-0.5)) < 1E-7);


    static constexpr double SQRT3_F64 = std::sqrt(3.0);

    static_assert(err_f64(math::sinpu(iq31(0.25 *(2.0 / 3))), iq31(SQRT3_F64 / 2)) < 1E-7);
    static_assert(err_f64(math::sinpu(iq31(0.25 *(4.0 / 3))), iq31(SQRT3_F64 / 2)) < 1E-7);
    static_assert(err_f64(math::sinpu(iq31(0.25 *(8.0 / 3))), iq31(-SQRT3_F64 / 2)) < 1E-7);
    static_assert(err_f64(math::sinpu(iq31(0.25 *(10.0 / 3))), iq31(-SQRT3_F64 / 2)) < 1E-7);


    static_assert(math::sinpu(iq30(0)).to_bits() == iq30(0).to_bits());
    static_assert(math::sinpu(iq30(0.25)).to_bits() == std::numeric_limits<iq31>::max().to_bits());
    static_assert(math::sinpu(iq30(0.5)).to_bits() == iq30(0).to_bits());
    static_assert(math::sinpu(iq30(1.0)).to_bits() == iq30(0).to_bits());

}

}