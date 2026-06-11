#include "setup_test.hpp"
#include "../fxmath/div.hpp"

namespace {


[[maybe_unused]] static void test_div32u(){
    static_assert(div32u<1>(65536 << 1, 1 << 1) == 65536 << 1);
    static_assert(div32u<10>(65536 << 10, 1 << 10) == 65536 << 10);
    static_assert(div32u<10>(32768 << 10, 1 << 10) == 32768 << 10);
    static_assert(div32u<16>(32768 << 16, 1 << 16) == 32768u << 16);
    static_assert(div32u<16>(20000u << 16, 1u << 16) == 20000u << 16);
    static_assert(div32u<5>(32768 << 5, 1 << 5) == 32768 << 5);

    static_assert(div32u<0>(32768 << 0, 1 << 0) == 32768 << 0);

    static_assert(abs_err(0.25f, (float)(uq32::from_bits(1000 * 8) / uq32::from_bits(32000))) <= 1e-4);
    static_assert(abs_err(0.5f, (float)(uq32::from_bits(1000 * 16) / uq32::from_bits(32000))) <= 1e-4);
    static_assert(abs_err(0.75f, (float)(uq32::from_bits(1000 * 24) / uq32::from_bits(32000))) <= 1e-4);
    // static_assert(div32u<32>(
    //     std::numeric_limits<uint32_t>::max(), 
    //     std::numeric_limits<uint32_t>::max()) 
    //     == std::numeric_limits<uint32_t>::max());
}

[[maybe_unused]] static void test_div32i(){
    static_assert(div32i<1>(65536 << 1, -1 << 1) == -65536 << 1);
    static_assert(div32i<10>(65536 << 10, -1 << 10) == -65536 << 10);
    static_assert(div32i<10>(32768 << 10, -1 << 10) == -32768 << 10);
    static_assert(div32i<16>(20000 << 16, -1 << 16) == -20000 << 16);
    static_assert(div32i<5>(32768 << 5, -1 << 5) == -32768 << 5);

    static_assert(div32i<0>(32768 << 0, -1 << 0) == -32768 << 0);
    static_assert(div32i<31>(
        std::numeric_limits<int32_t>::max(), 
        std::numeric_limits<int32_t>::min()) 
        == std::numeric_limits<int32_t>::min());
}

#if 0
// static constexpr uint32_t fit_table(uint32_t x){
//     return 2u * 64u * 64u * 127u / (63u * (2u * x) + 2u * 64u * 64u);
// }

static constexpr uint32_t fit_table(uint32_t x){
    return 4u * 64u * 64u * 127u / (63u * (4u * x - 1) + 4u * 64u * 64u);
}

[[maybe_unused]] static void test_fit_table(){

    static_assert(fit_table(0) == IQ6DIV_LOOPUP[0]);
    static_assert(fit_table(1) == IQ6DIV_LOOPUP[1]);
    static_assert(fit_table(2) == IQ6DIV_LOOPUP[2]);
    static_assert(fit_table(3) == IQ6DIV_LOOPUP[3]);
    static_assert(fit_table(4) == IQ6DIV_LOOPUP[4]);

    auto test_all = []() -> int32_t {
        for(uint32_t i = 0; i < 64; i++){
            if(fit_table(i) != IQ6DIV_LOOPUP[i]) return i;
        }
        return -1;
    };

    static_assert(test_all() == -1);
}
#endif

}