#include "mc_math.hpp"
#include "../motor_prelude.hpp"

namespace ymd::remnfoc{


static constexpr iiq32 calc_home_abs_position64(
    HomeMethod home_method, 
    uq32 encoder_initial_abs_position32, //编码器初始的单圈绝对位置
    uq32 home_abs_offset_position32 //原点的单圈绝对位置
){
    const auto enc_abs_postion32 = encoder_initial_abs_position32;
    const auto home_abs_offset32 = home_abs_offset_position32;
    const auto home_abs_offset64 = iiq32::from_bits(home_abs_offset32.to_bits()) ;

    switch(home_method){
        case HomeMethod::Initial:{
            return iiq32::from_bits(enc_abs_postion32.to_bits());
            break;
        }
        case HomeMethod::NearestZero:{
            const int32_t revs = (enc_abs_postion32 >> 31) ? 1 : 0;
            return iiq32(revs);
            break;
        }
        case HomeMethod::CeilZero:{
            const int32_t revs = (enc_abs_postion32.to_bits()) ? 1 : 0;
            return iiq32(revs);
            break;
        }
        case HomeMethod::FloorZero:{
            const int32_t revs = 0;
            return iiq32(revs);
            break;
        }
        case HomeMethod::Nearest:{
            constexpr int64_t HALF = int64_t(INT32_MAX);
            constexpr int64_t NEG_HALF = int64_t(INT32_MIN);
            const int64_t diff = int64_t(enc_abs_postion32.to_bits()) - int64_t(home_abs_offset32.to_bits());

            const int32_t adjust_revs = [&]{
                if(diff > HALF) return 1;
                else if(diff < NEG_HALF) return -1;
                return 0;
            }();

            return iiq32(adjust_revs) + home_abs_offset64;

            break;
        }
        case HomeMethod::Ceil:{
            const int32_t adjust_revs = [&]{
                if(enc_abs_postion32.to_bits() > home_abs_offset32.to_bits()) 
                    return 1;
                return 0;
            }();
            return iiq32(adjust_revs) + home_abs_offset64;
            break;
        }
        case HomeMethod::Floor:{
            const int32_t adjust_revs = [&]{
                if(enc_abs_postion32.to_bits() < home_abs_offset32.to_bits()) 
                    return - 1;
                return 0;
            }();
            return iiq32(adjust_revs) + home_abs_offset64;
            break;
        }
    }

    __builtin_unreachable();
    return 0;
};



}