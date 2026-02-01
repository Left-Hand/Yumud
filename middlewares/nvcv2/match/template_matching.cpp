#include "template_matching.hpp"
#include "core/math/realmath.hpp"

#include "middlewares/nvcv2/pixels/pixels.hpp"
#include <bits/stl_numeric.h>
#include "core/math/fastmath/sqrt_f.hpp"
#include "core/math/fastmath/sqrt_i.hpp"


#define BOUNDARY_CHECK()\
if(not math::Rect2u::from_size(src.size()).contains(math::Rect2u{offset, tmp.size()})){\
    ASSERT(false, "template_match: out of bound");\
    return 0;\
}\

template<iterable T>
using Itpair = std::pair<T, T>;


namespace ymd::nvcv2::match{

iq16 template_match(
    const Image<Binary> & src, 
    const Image<Binary> & tmp,
    const math::Vec2u & offset
){
    BOUNDARY_CHECK()

    const auto may_size = math::Rect2u(offset, tmp.size())
        .intersection(math::Rect2u::from_size(src.size()));
    
    if(may_size.is_none()) return 0;
    const auto size = may_size.unwrap();

    size_t score = 0;
    for(size_t y = 0; y < size.h(); y++){
        const Binary * p_tmp = &tmp[math::Vec2u16{
            static_cast<uint16_t>(0),
            static_cast<uint16_t>(y)
        }];
        const Binary * p_src = src.head_ptr() + src.position_to_index(math::Vec2u(0,y) + offset);
        for(size_t x = 0; x < size.w(); x++){
            score += int32_t((*p_tmp).is_white() ^ (*p_src).is_white());
            p_tmp++;
            p_src++;
        }
    }

    return score / size.area();
}

iq16 template_match_ncc(
    const Image<Gray> & src, 
    const Image<Gray> & tmp, 
    const math::Vec2u & offset
){
    BOUNDARY_CHECK()

    int32_t t_mean = pixels::mean(tmp).to_u8();
    int32_t s_mean = pixels::mean(src, math::Rect2u(offset, tmp.size())).to_u8();

    int64_t num = 0;
    uint64_t den_t = 0;
    uint64_t den_s = 0;

    for(auto y = 0u; y < tmp.size().y; y++){
        const Gray * p_tmp = &tmp[math::Vec2u{0,y}];
        const Gray * p_src = &src[math::Vec2u{0,y} + offset];

        int32_t line_num = 0;

        for(auto x = 0u; x < tmp.size().x; x++){
            int32_t tmp_val = int32_t(p_tmp->to_u8()) - t_mean;
            int32_t src_val = int32_t(p_src->to_u8()) - s_mean;

            line_num += ((tmp_val * src_val));
            den_t += math::square(tmp_val);
            den_s += math::square(src_val);

            p_tmp++;
            p_src++;
        }

        num += line_num;
    }

    
    if(num == 0) return 0;
    if(den_t == 0 || den_s == 0) return 0;

    const int64_t den = fast_sqrt_i(den_t) * fast_sqrt_i(den_s);
    return iq16::from_bits(num * 65535 / den);
}


iq16 template_match_squ(const Image<Gray> & src, const Image<Gray> & tmp, const math::Vec2u & offset){

    BOUNDARY_CHECK();

    uint64_t num = 0;
    uint32_t area = tmp.size().x * tmp.size().y;

    for(auto y = 0u; y < tmp.size().y; y++){
        const auto * p_tmp = &tmp[math::Vec2u{0,y}];
        const auto * p_src = &src[math::Vec2u{0,y} + offset];

        uint32_t line_num = 0;
        for(auto x = 0u; x < tmp.size().x; x++){
            int32_t tmp_val = p_tmp->to_u8();
            int32_t src_val = p_src->to_u8();

            line_num += math::square(tmp_val - src_val);

            p_tmp++;
            p_src++;
        }

        num += line_num;
    }

    
    if(num == 0) return 0;

    // iq16 ret = 0;
    uint16_t res = num / area;
    return 1 - iq16::from_bits(res);
}

iq16 template_match(const Image<Gray> & src, const Image<Gray> & tmp, const math::Vec2u & offset){
    return template_match_ncc(src, tmp, offset);
    // return template_match_squ(src, tmp, offs);
}

}