#include "setup_test.hpp"
#include "../fxmath/exp.hpp"

namespace {

[[maybe_unused]] static void test_exp(){
    {
        constexpr float f0 = float(math::exp(iq12(0)));
        constexpr float f0_5 = float(math::exp(iq12(0.5)));
        constexpr float f1 = float(math::exp(iq12(1)));
        constexpr float f3 = float(math::exp(iq12(3)));
        constexpr float f5 = float(math::exp(iq12(5)));
        constexpr float f7 = float(math::exp(iq12(7)));
        constexpr float f9 = float(math::exp(iq12(9)));

        static_assert(f0 == 1.0);
        static_assert(std::abs(f0_5 - 1.6487212707) < 1e-4);
        static_assert(std::abs(f1 - 2.7182818284590451) < 1e-4);
        static_assert(std::abs(f3 - 20.0855369232) < 1e-4);
        static_assert(std::abs(f5 - 148.413159103) < 1e-4);
        static_assert(std::abs(f7 - 1096.63315843) < 2e-4);
        static_assert(std::abs(f9 - 8103.08392758) < 1e-4);
    }

    {
        constexpr float f0 = float(math::exp(iq16(0)));
        constexpr float f0_5 = float(math::exp(iq16(0.5)));
        constexpr float f1 = float(math::exp(iq16(1)));
        constexpr float f3 = float(math::exp(iq16(3)));
        constexpr float f5 = float(math::exp(iq16(5)));
        constexpr float f7 = float(math::exp(iq16(7)));
        constexpr float f9 = float(math::exp(iq16(9)));

        static_assert(f0 == 1.0);
        static_assert(std::abs(f0_5 - 1.6487212707) < 1e-4);
        static_assert(std::abs(f1 - 2.7182818284590451) < 1e-4);
        static_assert(std::abs(f3 - 20.0855369232) < 1e-4);
        static_assert(std::abs(f5 - 148.413159103) < 1e-4);
        static_assert(std::abs(f7 - 1096.63315843) < 1e-4);
        static_assert(std::abs(f9 - 8103.08392758) < 1e-4);
    }

    {
        constexpr float f0 = float(math::exp(iq22(0)));
        constexpr float f0_5 = float(math::exp(iq22(0.5)));
        constexpr float f1 = float(math::exp(iq22(1)));
        constexpr float f3 = float(math::exp(iq22(3)));
        constexpr float f5 = float(math::exp(iq22(5)));
        constexpr float f_n3 = float(math::exp(iq22(-3)));
        constexpr float f_n5 = float(math::exp(iq22(-5)));

        static_assert(f0 == 1.0);
        static_assert(std::abs(f0_5 - 1.6487212707) < 1e-4);
        static_assert(std::abs(f1 - 2.7182818284590451) < 1e-4);
        static_assert(std::abs(f3 - 20.0855369232) < 1e-4);
        static_assert(std::abs(f5 - 148.413159103) < 1e-4);

        static_assert(std::abs(f_n3 - 0.0497870683679) < 1e-4);
        static_assert(std::abs(f_n5 - 0.00673794699909) < 1e-4);
    }
}


}