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



template<size_t Q, bool IS_SIGNED>
__attribute__((const, optimize( "-Ofast" )))
constexpr int32_t _iqn_div_impl(int32_t iqNInput1, int32_t iqNInput2)
{
    #if 0
    size_t ui8Index = 0;
    bool is_neg = 0;
    uint32_t uiq30Guess;
    uint32_t uiqNInput1;
    uint32_t uiq31Input2;
    uint32_t uiqNResult;
    uint64_t uiiqNInput1;

    if constexpr(IS_SIGNED == true) {
        /* save sign of denominator */
        if (iqNInput2 <= 0) {
            /* check for divide by zero */
            if (iqNInput2 == 0) {
                return INT32_MAX;
            } else {
                is_neg = 1;
                iqNInput2 = -iqNInput2;
            }
        }

        /* save sign of numerator */
        if (iqNInput1 < 0) {
            is_neg = !is_neg;
            iqNInput1 = -iqNInput1;
        }
    } else {
        /* Check for divide by zero */
        if (iqNInput2 == 0) {
            return INT32_MAX;
        }
    }

    /* Save input1 and input2 to unsigned IQN and IIQN (64-bit). */
    uiiqNInput1 = (uint64_t)iqNInput1;
    uiq31Input2 = (uint32_t)iqNInput2;

    /* Scale inputs so that 0.5 <= uiq31Input2 < 1.0. */
    while (uiq31Input2 < 0x40000000) {
        uiq31Input2 <<= 1;
        uiiqNInput1 <<= 1;
    }

    /*
     * Shift input1 back from iq31 to iqN but scale by 2 since we multiply
     * by result in iq30 format.
     */
    if constexpr(Q < 31) {
        uiiqNInput1 >>= (31 - Q - 1);
    } else {
        uiiqNInput1 <<= 1;
    }

    /* Check for saturation. */
    if (uiiqNInput1 >> 32) {
        if (is_neg) {
            return INT32_MIN;
        } else {
            return INT32_MAX;
        }
    } else {
        uiqNInput1 = (uint32_t)uiiqNInput1;
    }

    #else
    bool is_neg = 0;
    uint32_t uiqNResult;

    if constexpr(IS_SIGNED == true) {
        /* save sign of denominator */
        if (iqNInput2 == 0) [[unlikely]]{
            return INT32_MAX;
        }else if(iqNInput2 < 0){
            if(iqNInput2 == INT32_MIN) [[unlikely]] {
                iqNInput2 = INT32_MAX;
                is_neg = 1;
            }else{
                iqNInput2 = -iqNInput2;
                is_neg = 1;
            }
        }

        /* save sign of numerator */
        if (iqNInput1 < 0) {
            is_neg = !is_neg;

            if(iqNInput1 == INT32_MIN) [[unlikely]] {
                iqNInput1 = INT32_MAX;
            }else{
                iqNInput1 = -iqNInput1;
            }
        }

    } else {
        /* Check for divide by zero */
        if (iqNInput2 == 0) [[unlikely]] {
            return INT32_MAX;
        }
    }


    /* Scale inputs so that 0.5 <= uiq32Input2 < 1.0. */
    // Handle zero case to avoid undefined behavior in __builtin_clz
    // Find the number of leading zeros to determine the shift amount
    #if 0
    //1.046us per call @ch32v303 144mhz(fpu present)
    #if 0
    const size_t shift_amount = [&] -> size_t __no_inline{
        return size_t(CLZ(iqNInput2));
    }();
    #else
    const size_t shift_amount = size_t(CLZ(iqNInput2));
    #endif
    #else
    //0.79us per call @ch32v303 144mhz(fpu present)
    const size_t shift_amount = __builtin_clz(iqNInput2);
    #endif

    if(shift_amount >= 32) __builtin_unreachable();
    
    uint32_t uiq32Input2 = iqNInput2 << shift_amount;
    uint64_t uiiqNInput1 = uint64_t(iqNInput1);
    if constexpr(Q < 31) {
        const int32_t shifts = (31 - Q - shift_amount);
        if(shifts >= 0) {
            uiiqNInput1 >>= shifts;
        } else {
            uiiqNInput1 <<= -shifts;
        }
    } else {
        uiiqNInput1 <<= (Q - 31) + shift_amount;
    }

    size_t ui8Index = 0;
    uint32_t uiq30Guess;
    uint32_t uiqNInput1 = (uint32_t)uiiqNInput1;
    uint32_t uiq31Input2 = uiq32Input2 >> 1;
    // uint32_t uiqNResult;
    // uint64_t uiiqNInput1;
    #endif

    /* use left most 7 bits as ui8Index into lookup table (range: 32-64) */
    ui8Index = uiq31Input2 >> 24;
    ui8Index -= 64;
    uiq30Guess = (uint32_t)IQ6DIV_LOOPUP[ui8Index] << 24;



    /* 1st iteration */
    uint32_t ui30Temp = intrinsics::__mpyf_ul(uiq30Guess, uiq31Input2);
    ui30Temp = -((uint32_t)ui30Temp - 0x80000000);
    uiq30Guess = intrinsics::__mpyf_ul_reuse_arg1(uiq30Guess, ui30Temp << 1);

    /* 2nd iteration */
    ui30Temp = intrinsics::__mpyf_ul(uiq30Guess, uiq31Input2);
    ui30Temp = -((uint32_t)ui30Temp - 0x80000000);
    uiq30Guess = intrinsics::__mpyf_ul_reuse_arg1(uiq30Guess, ui30Temp << 1);

    /* 3rd iteration */
    ui30Temp = intrinsics::__mpyf_ul(uiq30Guess, uiq31Input2);
    ui30Temp = -((uint32_t)ui30Temp - 0x80000000);
    uiq30Guess = intrinsics::__mpyf_ul_reuse_arg1(uiq30Guess, ui30Temp << 1);

    /* Multiply 1/uiq31Input2 and uiqNInput1. */
    uiqNResult = intrinsics::__mpyf_ul(uiq30Guess, uiqNInput1);


    /* Saturate, add the sign and return. */
    if constexpr(IS_SIGNED == true) {
        if (uiqNResult > INT32_MAX) {
            if (is_neg) {
                return INT32_MIN;
            } else {
                return INT32_MAX;
            }
        } else {
            if (is_neg) {
                return -(int32_t)uiqNResult - 1;
            } else {
                return (int32_t)uiqNResult + 1;
            }
        }
    } else {
        return uiqNResult + 1;
    }
}

template<size_t Q>
__attribute__((const, optimize("-Ofast" )))
constexpr int32_t _div32i(int32_t iqNInput1, int32_t iqNInput2){
    return _iqn_div_impl<Q, true>(iqNInput1, iqNInput2);
}

template<size_t Q>
__attribute__((const, optimize("-Ofast" )))
constexpr uint32_t _div32u(uint32_t iqNInput1, uint32_t iqNInput2){
    return std::bit_cast<uint32_t>(_iqn_div_impl<Q, false>(
        std::bit_cast<int32_t>(iqNInput1), 
        std::bit_cast<int32_t>(iqNInput2)
    ));
}


}