#include "setup_test.hpp"
#include "../fxmath/sqrt.hpp"

namespace {


[[maybe_unused]] static void test_sqrt(){


    {

        // static constexpr uint32_t b1 = fxmath::details::sqrt32u<32>(uq32::from_bits(1u << (32-2))) .to_bits();
        // static constexpr uint32_t b2 = fxmath::details::sqrt32u<32>(uq32::from_bits(1u << (32-4))) .to_bits();
        // static_assert(b1
        //     == uq32::from_bits(1u << (32-1)).to_bits());
        // static_assert(b2
        //     == uq32::from_bits(1u << (32-2)).to_bits());
    }

    {

        static constexpr uint32_t b1 = fxmath::details::sqrt32u<32>(uq32::from_bits(1u << (32-16))) .to_bits();
        static constexpr uint32_t b2 = fxmath::details::sqrt32u<32>(uq32::from_bits(1u << (32-18))) .to_bits();
        static_assert(b1
            == uq32::from_bits(1u << (32-8)).to_bits());
        static_assert(b2
            == uq32::from_bits(1u << (32-9)).to_bits());
    }

    static_assert(math::sqrt(iq16(0)) == iq16(0));
    static_assert(math::sqrt(iq16(4)) == iq16(2));
    static_assert(math::sqrt(iq16(16)) == iq16(4));
    static_assert(math::sqrt(iq16(64)) == iq16(8));
    static_assert(math::sqrt(iq16(128 * 128)) == iq16(128));

    static_assert(math::sqrt(uq16(0)) == uq16(0));
    static_assert(math::sqrt(uq16(4)) == uq16(2));
    static_assert(math::sqrt(uq16(16)) == uq16(4));
    static_assert(math::sqrt(uq16(64)) == uq16(8));
    static_assert(math::sqrt(std::numeric_limits<uq16>::max()) == uq16(256));

    static_assert(math::sqrt(iiq16(0)) == iq16(0));
    static_assert(math::sqrt(iiq16(4)) == iq16(2));
    static_assert(math::sqrt(iiq16(16)) == iq16(4));
    static_assert(math::sqrt(iiq16(64)) == iq16(8));
    static_assert(math::sqrt(iiq16(36)) == iq16(6));
    static_assert(math::sqrt(iiq16(16)) == iq16(4));

    static_assert(err64(math::sqrt(uq32(0.25)).to_bits(), uq32(0.5).to_bits()) <= 4);
    static_assert(err64(math::sqrt(uq32(1.0/16)).to_bits(), uq32(1.0/4).to_bits()) <= 4);
    static_assert(err64(math::sqrt(uq32(1.0/64)).to_bits(), uq32(1.0/8).to_bits()) <= 4);
    static_assert(err64(math::sqrt(uq32(1.0/256)).to_bits(), uq32(1.0/16).to_bits()) <= 4);

    static_assert(err64(math::sqrt(iq20(0.25)).to_bits(), iq20(0.5).to_bits()) <= 4);
    static_assert(err64(math::sqrt(iq20(1.0/16)).to_bits(), iq20(1.0/4).to_bits()) <= 4);
    static_assert(err64(math::sqrt(iq20(1.0/64)).to_bits(), iq20(1.0/8).to_bits()) <= 4);
    static_assert(err64(math::sqrt(iq20(1.0/256)).to_bits(), iq20(1.0/16).to_bits()) <= 4);

    static_assert(math::inv_sqrt(iq10(16)) == iq10(0.25));
    static_assert(math::inv_sqrt(iq16(16)) == iq16(0.25));
    static_assert(math::inv_sqrt(uq16(16)) == uq16(0.25));

    static_assert(math::mag(iq2(3), iq2(4)) == iq2(5));
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
    static_assert(math::mag(iq16(3), iq16(4), iq16(12)) == iq16(13));
    static_assert(math::mag(iq16(1), iq16(1), iq16(1), iq16(1)) == iq16(2));

    static_assert(math::inv_mag(iq16(1), iq16(2), iq16(2)).to_bits() == iq16(1.0/3).to_bits());
}

}