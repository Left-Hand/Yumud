#include "core/math/real.hpp"
#include "core/math/batch/conv.hpp"
#include "core/math/iq/fixed_t.hpp"


#include "types/vectors/vector3.hpp"
#include "types/vectors/quat.hpp"

template<arithmetic T>
constexpr
auto build_norm_vec3_from_quat(
    const T x, const T y, const T z, const T w){

    return Vec3(
        Norm(2 * (qconv(Nrom_t(x) , Norm(z) , Norm(- w), Nrom_t(y)))),
        Norm(2 * (qconv(Nrom_t(w) , Norm(x) , Norm(y  ), Nrom_t(z)))),
        Norm(fconv(qsqu(Norm(w)) - qsqu(Norm(x)) - qsqu(Norm(y)) + qsau(Norm(z))))
    );
}

using namespace ymd;

void test_q14(){
    constexpr iq14 a = 1_r;
    constexpr iq14 b = 0.5_r;
    constexpr auto c = a * b;

    static_assert(c == 0.5_r);
}   

#if 0
void test_norm(){
    {
        constexpr iq14 a = 1_r;
        constexpr iq14 b = 0.5_r;
        constexpr auto na = Norm(a);
        constexpr auto nb = Norm(b);

        constexpr iq14 c = qmux(na, nb);

        static_assert(c == a * b);
    }

    {
        constexpr auto a1 = 0.5_q14;
        constexpr auto b1 = 0.5_q14;

        constexpr auto a2 = 0.5_q14;
        constexpr auto b2 = 0.5_q14;

        constexpr auto na1 = Norm(a1);
        constexpr auto nb1 = Norm(b1);

        constexpr auto na2 = Norm(a2);
        constexpr auto nb2 = Norm(b2);

        constexpr auto c = qconv(na1, nb1, na2, nb2);
        static_assert(c == a1 * b1 + a2 * b2);
    }

    {
        constexpr auto a1 = 0.5_iq16;
        constexpr auto b1 = 0.5_iq16;

        constexpr auto a2 = 0.5_iq16;
        constexpr auto b2 = 0.5_iq16;

        constexpr auto na1 = Norm(a1);
        constexpr auto nb1 = Norm(b1);

        constexpr auto na2 = Norm(a2);
        constexpr auto nb2 = Norm(b2);

        constexpr auto c = qconv(na1, nb1, na2, nb2);
        static_assert(c == a1 * b1 + a2 * b2);
    }
}
#endif