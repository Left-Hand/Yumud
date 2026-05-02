#include "fmtnum.hpp"


#include <array>

using namespace ymd;
using namespace ymd::str;





[[nodiscard]] char * str::fmtnum_integral64(
    char * p_str, 
    const uint64_t int_val, 
    uint8_t radix, 
    const IntTypeErased type
){


    const uint32_t high32 = static_cast<uint32_t>(int_val >> 32);
    const uint32_t low32 = static_cast<uint32_t>(int_val);


    switch(radix){
        case 10:
            //u32
            if(high32 == 0) [[likely]]{
                return fmtnum_integral32(p_str, static_cast<uint32_t>(int_val),  
                    radix, IntTypeErased::from<uint32_t>());
            }

            if(type.is_signed){
                //neg i32
                if(high32 == UINT32_MAX) [[likely]]{
                    p_str[0] = '-';
                    p_str++;
                    return fmtnum_integral32(p_str, static_cast<uint32_t>(-int_val), 
                        radix, IntTypeErased::from<uint32_t>());
                }
            }
            
            //TODO support 64bit
            return _fmtnum_mayneg_i32_dec(p_str, static_cast<uint32_t>(int_val), type);
        case 16:{
            constexpr size_t NUM_DIGITS_U32 = (sizeof(uint32_t) * 8) / 4;
            _fmtnum_u32_hex({p_str, p_str + NUM_DIGITS_U32}, high32);
            _fmtnum_u32_hex({p_str + NUM_DIGITS_U32, p_str + 2 * NUM_DIGITS_U32}, low32);
            return p_str + (NUM_DIGITS_U32 * 2);
        }
        case 8:
            // return _fmtnum_u32_oct(p_str, static_cast<uint32_t>(int_val));
            //TODO
            return p_str;
        case 2:{
            constexpr size_t NUM_DIGITS_U32 = (sizeof(uint32_t) * 8) / 1;
            _fmtnum_u32_hex({p_str, p_str + NUM_DIGITS_U32}, high32);
            _fmtnum_u32_hex({p_str + NUM_DIGITS_U32, p_str + 2 * NUM_DIGITS_U32}, low32);
            return p_str + (NUM_DIGITS_U32 * 2);
        }
        default:
            //no chars 
            return p_str;
    }
}

