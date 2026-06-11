
#include "setup_test.hpp"
#include "../fxmath/log.hpp"

namespace {



[[maybe_unused]] static void test_log(){
    {
        constexpr float f_n3 = float(math::ln(uq16(0.0497870683679)));
        constexpr float f_n2 = float(math::ln(uq16(0.135335283237)));
        constexpr float f_n1_25 = float(math::ln(uq16(0.28650479686)));
        constexpr float f_n1 = float(math::ln(uq16(0.367879441171)));
        constexpr float f0_25 = float(math::ln(uq16(1.28402541669)));
        constexpr float f0_5 = float(math::ln(uq16(1.6487212707)));
        constexpr float f0_75 = float(math::ln(uq16(2.11700001661)));
        constexpr float f0_95 = float(math::ln(uq16(2.58570965932)));
        constexpr float f1 = float(math::ln(uq16(2.7182818284590451)));
        constexpr float f3 = float(math::ln(uq16(20.0855369232)));
        static_assert(std::abs(f_n3 - -3) < 3e-4);
        static_assert(std::abs(f_n2 - -2) < 3e-4);
        static_assert(std::abs(f_n1_25 - -1.25) < 1e-4);
        static_assert(std::abs(f_n1 - -1) < 1e-4);
        static_assert(std::abs(f0_25 - 0.25) < 1e-4);
        static_assert(std::abs(f0_5 - 0.5) < 1e-4);
        static_assert(std::abs(f0_75 - 0.75) < 1e-4);
        static_assert(std::abs(f0_95 - 0.95) < 1e-4);
        static_assert(std::abs(f1 - 1.0) < 1e-4);
        static_assert(std::abs(f3 - 3.0) < 1e-4);
    }
    {
        constexpr float f_n2 = float(math::lg(uq16(0.01)));
        constexpr float f1 = float(math::lg(uq16(10)));
        constexpr float f3 = float(math::lg(uq16(1000)));
        constexpr float f4 = float(math::lg(uq10(10000)));
        constexpr float f5 = float(math::lg(uq10(100000)));

        static_assert(std::abs(f_n2 - -2.0) < 3e-4);
        static_assert(std::abs(f1 - 1.0) < 1e-4);
        static_assert(std::abs(f3 - 3.0) < 1e-4);
        static_assert(std::abs(f4 - 4.0) < 1e-3);
        static_assert(std::abs(f5 - 5.0) < 1e-3);
    }

}

}