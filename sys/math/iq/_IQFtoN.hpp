#pragma once

#include <stdint.h>

#include "support.h"

template<size_t Q>
requires (Q < 32)
__fast_inline scexpr _iq<Q> _IQFtoN(const float fv){
    if(std::is_constant_evaluated()){
        return _iq<Q>::from_i32(int32_t(fv * int(1 << Q)));
    }

    const int32_t d = std::bit_cast<int32_t>(fv);
    const int32_t exponent = ((d >> 23) & 0xff);
    const uint64_t mantissa = (exponent == 0) ? (0) : ((d & ((1 << 23) - 1)) | (1 << 23));

    const uint64_t temp = [&] -> uint64_t {
        if(exponent == 0 or exponent == 0xff){
            return 0;
        }else{
            return LSHIFT(mantissa, exponent - 127);
        }
    }();

    const uint64_t uresult = RSHIFT(temp, (23 - Q));
    const int32_t result = d > 0 ? uresult : -uresult;

    if((bool(d > 0) ^ bool(result > 0)) or (uresult > (uint64_t)0x80000000)){//OVERFLOW
        if(d > 0){
            return std::bit_cast<_iq<Q>>(0x7FFFFFFF);
        }else{
            return std::bit_cast<_iq<Q>>(0x80000000);
        }
    }else{
        return std::bit_cast<_iq<Q>>(result);
    }

}