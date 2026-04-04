#include "ms5611_prelude.hpp"

namespace {
using namespace ymd::drivers;


[[maybe_unused]] static void test(){
    static constexpr auto coeffs = MS5611_Prelude::Coeffs{
        .c_table = {
            40127, 36924, 23317, 23282, 33464, 28312
        }
    };

    constexpr uint32_t d1 = 9085466;
    constexpr uint32_t d2 = 8569150;

    constexpr auto inter = coeffs.calc_intermediate(d1, d2); 

    static_assert(inter.dt == 2366);
    static_assert(inter.temp == 2007);

    constexpr auto product = coeffs.calc_product(inter);

    // 2420281617LL, 1315097036LL, 100009L
    static_assert(product.off == 2420281617);
    static_assert(product.sens == 1315097036);
    static_assert(product.pressure == 100009);
}

}