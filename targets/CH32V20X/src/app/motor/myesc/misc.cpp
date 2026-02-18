#if 0

#ifndef M_PI
#define M_PI (3.1415926536)
#endif

#define TYPE_PU         (0)
#define TYPE_RAD        (1)


namespace ymd::myiqmath::details{

struct [[nodiscard]] Atan2Flag{
    /*
    * Extract the sign from the inputs and set the following flag bits:
    *
    *      flag = xxxxxTQS
    *      x = unused
    *      T = transform was inversed
    *      Q = 2nd or 3rd quadrant (-x)
    *      S = sign bit needs to be set (-y)
    */

    uint8_t y_is_neg:1;
    uint8_t x_is_neg:1;
    uint8_t inversed:1;

    static constexpr Atan2Flag zero(){
        return Atan2Flag{0, 0, 0};
    }


    template<size_t Q, int type>
    [[nodiscard]] constexpr int32_t apply_to_uiq32pu(uint32_t uiq32ResultPU) const {
        auto & self = *this;
        int32_t iqNResult;
        int32_t iq29Result;

        /* Check if we inversed the transformation. */
        if (self.inversed) {
            /* atan(y/x) = pi/2 - uiq32ResultPU */
            uiq32ResultPU = (uint32_t)(0x40000000 - uiq32ResultPU);
        }

        /* Check if the result needs to be mirrored to the 2nd/3rd quadrants. */
        if (self.x_is_neg) {
            /* atan(y/x) = pi - uiq32ResultPU */
            uiq32ResultPU = (uint32_t)(0x80000000 - uiq32ResultPU);
        }

        /* Round and convert result to correct format (radians/PU and iqN type). */
        if constexpr(type ==  TYPE_PU) {
            uiq32ResultPU += (uint32_t)1 << (31 - Q);
            iqNResult = uiq32ResultPU >> (32 - Q);
        }

        /* Set the sign bit and result to correct quadrant. */
        if (self.y_is_neg) {
            return -iqNResult;
        } else {
            return iqNResult;
        }
    };

    [[nodiscard]] constexpr uint8_t to_u8() const {
        return std::bit_cast<uint8_t>(*this);
    }
};

struct [[nodiscard]] Atan2Intermediate{
    using Self = Atan2Intermediate;


    // * Calculate atan2 using a 3rd order Taylor series. The coefficients are stored
    // * in a lookup table with 17 ranges to give an accuracy of XX bits.
    // *
    // * The input to the Taylor series is the ratio of the two inputs and must be
    // * in the range of 0 <= input <= 1. If the y argument is larger than the x
    // * argument we must apply the following transformation:
    // *
    // *     atan(y/x) = pi/2 - atan(x/y)
    // */
    static constexpr uint32_t transfrom_uq31_x_to_uq32_result(uint32_t uiq31Input) {
        const auto * piq32Coeffs = &iqmath::details::IQ32ATAN_COEFFS[(uiq31Input >> 24) & 0x00fc];
        /*
        * Calculate atan(x) using the following Taylor series:
        *
        *     atan(x) = ((c3*x + c2)*x + c1)*x + c0
        */

        /* c3*x */
        uint32_t uiq32ResultPU = iqmath::details::__mpyf_l(uiq31Input, *piq32Coeffs++);

        /* c3*x + c2 */
        uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;

        /* (c3*x + c2)*x */
        uiq32ResultPU = iqmath::details::__mpyf_l(uiq31Input, uiq32ResultPU);

        /* (c3*x + c2)*x + c1 */
        uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;

        /* ((c3*x + c2)*x + c1)*x */
        uiq32ResultPU = iqmath::details::__mpyf_l(uiq31Input, uiq32ResultPU);

        /* ((c3*x + c2)*x + c1)*x + c0 */
        uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;
        return uiq32ResultPU;
    }
};

std::tuple<Atan2Flag, uint32_t> convert_to_flag(uint32_t uiqNInputX, uint32_t uiqNInputY){
    Atan2Flag flag = Atan2Flag::zero();
    uint32_t uiq31Input;

    if (uiqNInputY & (1U << 31)) {
        flag.y_is_neg = 1;
        uiqNInputY = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uiqNInputY));
    }

    if (uiqNInputX & (1U << 31)) {
        flag.x_is_neg = 1;
        uiqNInputX = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uiqNInputX));
    }

    /*
    * Calcualte the ratio of the inputs in iq31. When using the iq31 div
    * fucntions with inputs of matching type the result will be iq31:
    *
    *     iq31 = _IQ31div(iqN, iqN);
    */
    if (uiqNInputX < uiqNInputY) {
        flag.inversed = 1;
        uiq31Input = std::bit_cast<uint32_t>(iqmath::details::__IQNdiv_impl<31, true>(
            uiqNInputX, uiqNInputY));
    } else if((uiqNInputX > uiqNInputY)) {
        uiq31Input = std::bit_cast<uint32_t>(iqmath::details::__IQNdiv_impl<31, true>(
            uiqNInputY, uiqNInputX));
    } else{
        // 1/8 lap
        // 1/8 * 2^32
        // return flag.template apply_to_uiq32pu<Q, type>(((1u << (32 - 3))));
        uiq31Input = (1u << (32 - 3));
    }
    return std::make_tuple(flag, uiq31Input);
}

template<const size_t Q, const uint8_t type>
constexpr int32_t __IQNatan2_impl(uint32_t uiqNInputY, uint32_t uiqNInputX){
    const auto [flag, uiq31Input] = convert_to_flag(uiqNInputX, uiqNInputY);
    
    const uint32_t uiq32ResultPU = Atan2Intermediate::transfrom_uq31_x_to_uq32_result(uiq31Input);
    return flag.template apply_to_uiq32pu<Q, type>(uiq32ResultPU);
}

template<const size_t Q>
constexpr math::fixed<Q, int32_t> test_atan(
    math::fixed<Q, int32_t> iqNInputY, 
    math::fixed<Q, int32_t> iqNInputX)
{
    return math::fixed<Q, int32_t>::from_bits(__IQNatan2_impl<Q, TYPE_RAD>(
        std::bit_cast<uint32_t>(iqNInputY.to_bits()), 
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );

    // Atan2Intermediate::transfrom_uq31_x_to_uq32_result
}
template<const size_t Q, const uint8_t type>
constexpr int32_t __IQNatan_impl(uint32_t uiqNInputX){
    return __IQNatan2_impl<Q, type>(uiqNInputX, (1u << Q));
}


template<const size_t Q>
constexpr math::fixed<Q, int32_t> _IQNatan2(
    math::fixed<Q, int32_t> iqNInputY, 
    math::fixed<Q, int32_t> iqNInputX
){
    return math::fixed<Q, int32_t>::from_bits(__IQNatan2_impl<Q, TYPE_RAD>(
        std::bit_cast<uint32_t>(iqNInputY.to_bits()), 
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

template<const size_t Q>
constexpr math::fixed<Q, int32_t> _IQNatan2PU(
    math::fixed<Q, int32_t> iqNInputY, 
    math::fixed<Q, int32_t> iqNInputX
){
    return math::fixed<Q, int32_t>::from_bits(__IQNatan2_impl<Q, TYPE_PU>(
        std::bit_cast<uint32_t>(iqNInputY.to_bits()), 
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

template<const size_t Q>
constexpr math::fixed<Q, int32_t> _IQNatan2(math::fixed<Q, int32_t> iqNInputX){
    return math::fixed<Q, int32_t>::from_bits(__IQNatan_impl<Q, TYPE_RAD>(
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

template<const size_t Q>
constexpr math::fixed<Q, int32_t> _IQNatan2PU(math::fixed<Q, int32_t> iqNInputX){
    return math::fixed<Q, int32_t>::from_bits(__IQNatanPU_impl<Q, TYPE_PU>(
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

}
#endif