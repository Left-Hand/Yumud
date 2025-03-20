#include "core/math/real.hpp"
#include "core/math/fast/conv.hpp"
#include "core/math/iq/iq_t.hpp"


#include "types/vector3/Vector3.hpp"
#include "types/quat/Quat.hpp"

template<arithmetic T>
constexpr
auto build_norm_vec3_from_quat(
    const T x, const T y, const T z, const T w){

    return Vector3_t(
        Norm_t(2 * (qconv(Nrom_t(x) , Norm_t(z) , Norm_t(- w), Nrom_t(y)))),
        Norm_t(2 * (qconv(Nrom_t(w) , Norm_t(x) , Norm_t(y  ), Nrom_t(z)))),
        Norm_t(fconv(qsqu(Norm_t(w)) - qsqu(Norm_t(x)) - qsqu(Norm_t(y)) + qsau(Norm_t(z))))
    );
}

using namespace ymd;

void test_q14(){
    constexpr q14 a = 1_r;
    constexpr q14 b = 0.5_r;
    constexpr auto c = a * b;

    static_assert(c == 0.5_r);
}   

void test_norm(){
    {
        constexpr q14 a = 1_r;
        constexpr q14 b = 0.5_r;
        constexpr auto na = Norm_t(a);
        constexpr auto nb = Norm_t(b);

        constexpr q14 c = qmux(na, nb);

        static_assert(c == a * b);
    }

    {
        constexpr auto a1 = 0.5_q14;
        constexpr auto b1 = 0.5_q14;

        constexpr auto a2 = 0.5_q14;
        constexpr auto b2 = 0.5_q14;

        constexpr auto na1 = Norm_t(a1);
        constexpr auto nb1 = Norm_t(b1);

        constexpr auto na2 = Norm_t(a2);
        constexpr auto nb2 = Norm_t(b2);

        constexpr auto c = qconv(na1, nb1, na2, nb2);
        static_assert(c == a1 * b1 + a2 * b2);
    }

    {
        constexpr auto a1 = 0.5_q16;
        constexpr auto b1 = 0.5_q16;

        constexpr auto a2 = 0.5_q16;
        constexpr auto b2 = 0.5_q16;

        constexpr auto na1 = Norm_t(a1);
        constexpr auto nb1 = Norm_t(b1);

        constexpr auto na2 = Norm_t(a2);
        constexpr auto nb2 = Norm_t(b2);

        constexpr auto c = qconv(na1, nb1, na2, nb2);
        static_assert(c == a1 * b1 + a2 * b2);
    }
}
