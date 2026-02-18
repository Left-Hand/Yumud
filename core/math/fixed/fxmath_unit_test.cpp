#include "fxmath.hpp"
#include <cmath>

using namespace ymd;
using namespace ymd::fxmath;
using namespace ymd::literals;

namespace{

static constexpr int64_t err64(const uint64_t a, const uint64_t b){
    if(a > b) return a - b;
    else return b - a;
}

[[maybe_unused]] void test_sqrt(){
    static_assert(math::sqrt(iq16(0)) == iq16(0));
    static_assert(math::sqrt(iq16(4)) == iq16(2));
    static_assert(math::sqrt(iq16(16)) == iq16(4));
    static_assert(math::sqrt(iq16(64)) == iq16(8));
    static_assert(math::sqrt(iq16(36)) == iq16(6));
    static_assert(math::sqrt(iq16(16)) == iq16(4));

    static_assert(math::sqrt(iiq16(0)) == iq16(0));
    static_assert(math::sqrt(iiq16(4)) == iq16(2));
    static_assert(math::sqrt(iiq16(16)) == iq16(4));
    static_assert(math::sqrt(iiq16(64)) == iq16(8));
    static_assert(math::sqrt(iiq16(36)) == iq16(6));
    static_assert(math::sqrt(iiq16(16)) == iq16(4));

    static_assert(math::sqrt(uq16(4)) == uq16(2));
    static_assert(math::sqrt(uq16(16)) == uq16(4));

    static_assert(err64(math::sqrt(uq32(0.25)).to_bits(), uq32(0.5).to_bits()) <= 4);
    static_assert(err64(math::sqrt(uq32(1.0/16)).to_bits(), uq32(1.0/4).to_bits()) <= 4);
    static_assert(err64(math::sqrt(uq32(1.0/64)).to_bits(), uq32(1.0/8).to_bits()) <= 4);
    static_assert(err64(math::sqrt(uq32(1.0/256)).to_bits(), uq32(1.0/16).to_bits()) <= 4);

    static_assert(math::inv_sqrt(iq10(16)) == iq10(0.25));
    static_assert(math::inv_sqrt(iq16(16)) == iq16(0.25));
    static_assert(math::inv_sqrt(uq16(16)) == uq16(0.25));

    static_assert(math::mag(iq16(3), iq16(4)) == iq16(5));
    static_assert(math::mag(iq26(3), iq26(4)) == iq26(5));

    static_assert(math::inv_mag(iq16(3), iq16(4)).to_bits() == iq16(0.2).to_bits());
    static_assert(math::inv_mag(iq16(12), iq16(5)).to_bits() == iq16(1.0/13).to_bits());

    static_assert(math::mag(iq16(3), iq16(4)) == iq16(5));
    static_assert(math::mag(iq26(3), iq26(4)) == iq26(5));

    static_assert(math::inv_mag(iq16(3), iq16(4)).to_bits() == iq16(0.2).to_bits());
    static_assert(math::inv_mag(iq16(12), iq16(5)).to_bits() == iq16(1.0/13).to_bits());

    static_assert(math::mag(iq16(0), iq16(0), iq16(0)) == iq16(0));
    static_assert(math::mag(iq16(1), iq16(2), iq16(2)) == iq16(3));
    static_assert(math::mag(iq16(3), iq16(4), iq16(0)) == iq16(5));
    static_assert(math::mag(iq16(1), iq16(1), iq16(1), iq16(1)) == iq16(2));

    static_assert(math::inv_mag(iq16(1), iq16(2), iq16(2)).to_bits() == iq16(1.0/3).to_bits());
}




[[maybe_unused]] static void test_sin(){
    static_assert(math::sinpu(iq16(0)) == 0);
    static_assert(math::sinpu(iq16(0.25)) == std::numeric_limits<iq31>::max());
    static_assert(math::sinpu(iq16(0.5)) == iq16(0));
    static_assert(math::sinpu(iq16(1.0)) == iq16(0));

    static_assert(math::sinpu(iq10(0)) == iq10(0));
    static_assert(math::sinpu(iq10(0.25)) == std::numeric_limits<iq31>::max());
    static_assert(math::sinpu(iq10(0.5)) == iq10(0));
    static_assert(math::sinpu(iq10(1.0)) == iq10(0));

    static_assert(math::sinpu(iq30(0)) == iq30(0));
    static_assert(math::sinpu(iq30(0.25)) == std::numeric_limits<iq31>::max());
    static_assert(math::sinpu(iq30(0.5)) == iq30(0));
    static_assert(math::sinpu(iq30(1.0)) == iq30(0));

}
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

[[maybe_unused]] static void test_atan(){
    static_assert(std::abs((double)math::atan2pu(ymd::literals::iq16(1.0), ymd::literals::iq16(1.0)) - 0.125) < 1E-7);
    static_assert(std::abs((double)math::atan2pu(-ymd::literals::iq16(1.0), -ymd::literals::iq16(1.0)) - 0.625) < 1E-7);
    static_assert(std::abs((double)math::atan2pu(ymd::literals::iq16(1.0), -ymd::literals::iq16(1.0)) - 0.375) < 1E-7);
    static_assert(std::abs((double)math::atan2pu(-ymd::literals::iq16(1.0), ymd::literals::iq16(1.0)) - 0.875) < 1E-7);

    static_assert(std::abs((double)math::atanpu(ymd::literals::iq16(1.0)) - 0.125) < 1E-7);
    static_assert(std::abs((double)math::atanpu(-ymd::literals::iq16(1.0)) - 0.875) < 1E-7);
}

[[maybe_unused]] static void test_asin(){
    static_assert(std::abs((double)math::asin(ymd::literals::iq16(1.0)) - (M_PI / 2)) < 1E-7);
    static_assert(std::abs((double)math::asin(-ymd::literals::iq16(1.0)) - (-M_PI / 2)) < 1E-7);
    
    static_assert(std::abs((double)math::asin(ymd::literals::iq16(0.5)) - (M_PI / 6)) < 1E-7);
}

}