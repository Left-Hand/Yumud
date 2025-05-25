#include "template.hpp"
#include "core/math/realmath.hpp"

#include "nvcv2/pixels/pixels.hpp"
#include <bits/stl_numeric.h>
#include "dsp/fastmath/sqrt.hpp"
#include "dsp/fastmath/square.hpp"


#define BOUNDARY_CHECK()\
if(not src.size().to_rect().contains(Rect2u{offs, tmp.size()})){\
    ASSERT(false, "template_match: out of bound");\
    return 0;\
}\

template<iterable T>
using Itpair = std::pair<T, T>;


namespace ymd::nvcv2::match{

real_t template_match(
    const Image<Binary> & src, 
    const Image<Binary> & tmp,
    const Vector2u & offs){
    BOUNDARY_CHECK()

    const auto size_opt = Rect2u(offs, tmp.size())
        .intersection(src.size().to_rect());
    
    if(size_opt.is_none()) return 0;
    const auto size = size_opt.unwrap();

    size_t score = 0;
    for(size_t y = 0; y < size.h(); y++){
        const auto * p_tmp = &tmp[Vector2u(0,y)];
        const auto * p_src = &src[Vector2u(0,y) + offs];
        for(size_t x = 0; x < size.w(); x++){
            score += int32_t(bool(*p_tmp) ^ bool(*p_src));
            p_tmp++;
            p_src++;
        }
    }

    return score / size.get_area();
}

real_t template_match_ncc(
    const Image<Grayscale> & src, 
    const Image<Grayscale> & tmp, 
    const Vector2u & offs
){
    BOUNDARY_CHECK()

    int32_t t_mean = int32_t(pixels::mean(tmp));
    int32_t s_mean = int32_t(pixels::mean(src, Rect2u(offs, tmp.size())));

    int64_t num = 0;
    uint64_t den_t = 0;
    uint64_t den_s = 0;

    for(auto y = 0u; y < tmp.size().y; y++){
        const Grayscale * p_tmp = &tmp[Vector2u{0,y}];
        const Grayscale * p_src = &src[Vector2u{0,y} + offs];

        int32_t line_num = 0;

        for(auto x = 0u; x < tmp.size().x; x++){
            int32_t tmp_val = int32_t(uint8_t(*p_tmp)) - t_mean;
            int32_t src_val = int32_t(uint8_t(*p_src)) - s_mean;

            line_num += ((tmp_val * src_val));
            den_t += square(tmp_val);
            den_s += square(src_val);

            p_tmp++;
            p_src++;
        }

        num += line_num;
    }

    
    if(num == 0) return 0;
    if(den_t == 0 || den_s == 0) return 0;

    const int64_t den = fast_sqrt_i(den_t) * fast_sqrt_i(den_s);
    return s16_to_uni(num * 65535 / den);
}


real_t template_match_squ(const Image<Grayscale> & src, const Image<Grayscale> & tmp, const Vector2u & offs){

    BOUNDARY_CHECK();

    uint64_t num = 0;
    uint32_t area = tmp.size().x * tmp.size().y;

    for(auto y = 0u; y < tmp.size().y; y++){
        const auto * p_tmp = &tmp[Vector2u{0,y}];
        const auto * p_src = &src[Vector2u{0,y} + offs];

        uint32_t line_num = 0;
        for(auto x = 0u; x < tmp.size().x; x++){
            int32_t tmp_val = *p_tmp;
            int32_t src_val = *p_src;

            line_num += FAST_SQUARE8(tmp_val - src_val);

            p_tmp++;
            p_src++;
        }

        num += line_num;
    }

    
    if(num == 0) return 0;

    real_t ret;
    uint16_t res = num / area;
    return 1 - u16_to_uni(res);
}

real_t template_match(const Image<Grayscale> & src, const Image<Grayscale> & tmp, const Vector2u & offs){
    return template_match_ncc(src, tmp, offs);
    // return template_match_squ(src, tmp, offs);
}

}