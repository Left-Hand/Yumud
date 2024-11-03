#include "template.hpp"
#include <bits/stl_numeric.h>
#include "../dsp/fastmath/sqrt.hpp"
#include "../dsp/fastmath/square.hpp"

template<iterable T>
using Itpair = std::pair<T, T>;

namespace std{
    template<iterable T>
    inline constexpr size_t std::distance(const Itpair<T> & pair){
        return std::distance(pair.first, pair.second);
    }
}

#define FAST_SQUARE(x) (x * x)

static auto mean(const Itpair<auto> & src){
    return std::accumulate(src.first, src.second, 0) / std::distance(src);
}

static auto stddev(const Itpair<auto> & src){
    auto m = mean(src);
    auto diff = std::transform(src.first, src.second, src.first, [m](auto val){return val - m;});
    return std::sqrt(std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0) / std::distance(src));
}

namespace yumud::nvcv2::Match{

real_t template_match(const Image<Binary> & src, const Image<Binary> & tmp, const Vector2i & offs){
    // auto rect = Rect2i(offs, tmp.get_size()).intersection(src.get_view());

    // uint and_score = 0;
    // uint or_score = 0;

    // for(uint y = 0; y < (uint)rect.h; y++){
    //     const auto * tmp_ptr = &tmp[Vector2i{0,y}];
    //     const auto * src_ptr = &src[Vector2i{0,y} + offs];
    //     for(uint x = 0; x < (uint)rect.w; x++){
    //         and_score += int(bool(*tmp_ptr) && bool(*src_ptr));
    //         or_score += int(bool(*src_ptr) || bool(*src_ptr));
    //     }
    // }

    // real_t ret;
    // uint16_t res = and_score * 65535 / or_score;
    // u16_to_uni(res, ret);
    // return ret;

    auto rect = Rect2i(offs, tmp.get_size()).intersection(src.get_view());

    uint score = 0;
    // uint base = tmp.sum() / 255;
    // uint or_score = 0;

    for(uint y = 0; y < (uint)rect.h; y++){
        const auto * tmp_ptr = &tmp[Vector2i(0,y)];
        const auto * src_ptr = &src[Vector2i(0,y) + offs];
        for(uint x = 0; x < (uint)rect.w; x++){
            score += int(bool(*tmp_ptr) ^ bool(*src_ptr));
            // if(bool(*tmp_ptr) + bool(*src_ptr) == 1) score++;
            tmp_ptr++;
            src_ptr++;
        }
    }

    // real_t ret;
    // uint16_t res = score * 65535 / base;
    // u16_to_uni(res, ret);
    // return 1-ret;

    // return base - score;
    return score;
}

#define BOUNDARY_CHECK()\
if(not src.get_view().contains(Rect2i{offs, tmp.get_size()})){\
    ASSERT_WITH_HALT(false, "template_match: out of bound");\
    return 0;\
}\


#define FAST_SQRT(x) (fast_sqrt_i(x))

real_t template_match_ncc(const Image<Grayscale> & src, const Image<Grayscale> & tmp, const Vector2i & offs){
    BOUNDARY_CHECK()

    int t_mean = int(tmp.mean());
    int s_mean = int(src.mean(Rect2i(offs, tmp.get_size())));

    int64_t num = 0;
    uint32_t den_t = 0;
    uint32_t den_s = 0;

    for(auto y = 0; y < tmp.get_size().y; y++){
        const auto * tmp_ptr = &tmp[Vector2i{0,y}];
        const auto * src_ptr = &src[Vector2i{0,y} + offs];

        int32_t line_num = 0;
        for(auto x = 0; x < tmp.get_size().x; x++){
            int32_t tmp_val = *tmp_ptr - t_mean;
            int32_t src_val = *src_ptr - s_mean;

            line_num += ((tmp_val * src_val));
            den_t += FAST_SQUARE(tmp_val);
            den_s += FAST_SQUARE(src_val);

            tmp_ptr++;
            src_ptr++;
        }

        num += line_num;
    }

    
    if(num == 0) return 0;
    if(den_t == 0 || den_s == 0) return sign(num);

    real_t ret;

    int64_t den = FAST_SQRT(den_t) * FAST_SQRT(den_s);
    uint16_t res = std::abs(num) * 65535 / den;
    u16_to_uni(res, ret);
    return ret;
}


real_t template_match_squ(const Image<Grayscale> & src, const Image<Grayscale> & tmp, const Vector2i & offs){

    BOUNDARY_CHECK();

    uint64_t num = 0;
    uint32_t area = tmp.get_size().x * tmp.get_size().y;

    for(auto y = 0; y < tmp.get_size().y; y++){
        const auto * tmp_ptr = &tmp[Vector2i{0,y}];
        const auto * src_ptr = &src[Vector2i{0,y} + offs];

        uint32_t line_num = 0;
        for(auto x = 0; x < tmp.get_size().x; x++){
            int32_t tmp_val = *tmp_ptr;
            int32_t src_val = *src_ptr;

            line_num += FAST_SQUARE8(tmp_val - src_val);

            tmp_ptr++;
            src_ptr++;
        }

        num += line_num;
    }

    
    if(num == 0) return 0;

    real_t ret;
    uint16_t res = num / area;
    u16_to_uni(res, ret);
    return 1-ret;

}

#undef FAST_SQUARE
#undef FAST_SQRT

real_t template_match(const Image<Grayscale> & src, const Image<Grayscale> & tmp, const Vector2i & offs){
    return template_match_ncc(src, tmp, offs);
    // return template_match_squ(src, tmp, offs);
}

}