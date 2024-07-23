#include "template.hpp"
#include <bits/stl_numeric.h>
// static real_t template_match_c_impl(const uint8_t * src, const uint8_t * dst, )

template<iterable T>
using Itpair = std::pair<T, T>;

namespace std{
    template<iterable T>
    inline constexpr size_t std::distance(const Itpair<T> & pair){
        return std::distance(pair.first, pair.second);
    }
}

static auto mean(const Itpair<auto> & src){
    return std::accumulate(src.first, src.second, 0) / std::distance(src);
}

static auto stddev(const Itpair<auto> & src){
    auto m = mean(src);
    auto diff = std::transform(src.first, src.second, src.first, [m](auto val){return val - m;});
    return std::sqrt(std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0) / std::distance(src));
}


static uint64_t integer_sqrt(uint64_t n) {
	if (n == 0 || n == 1) return n;
	
	uint64_t x = n;
	uint64_t y = (x + 1) / 2;
	while (y < x) {
		x = y;
		y = (x + n / x) / 2;
	}
	// Check if x is the correct square root
	if (x * x > n) {
		return x - 1;
	} else {
		return x;
	}
}


namespace NVCV2::Match{

real_t template_match(const Image<Binary> & src, const Image<Binary> & tmp, const Vector2i & offs){
    auto rect = Rect2i(offs, tmp.get_size()).intersection(src.get_window());
    size_t cnt = 0;
    size_t eff = 0;
    for(auto y = 0; y < rect.h; y++){
        for(auto x = 0; x < rect.w; x++){
            if(tmp[{x,y}]){
                cnt++;
                if(src[offs + Vector2i{x,y}]){
                    eff++;
                }
            }
        }
    }
    if(cnt == 0) return 0;

    real_t ret;
    u16_to_uni(((eff * 0XFFFF) / cnt), ret);
    return ret;
}

real_t template_match(const Image<Grayscale> & src, const Image<Grayscale> & tmp, const Vector2i & offs){
    static constexpr uint lossy_bits = 0;
    //boundary check 
    if(not src.get_window().contains(Rect2i{offs, tmp.get_size()})){
        // ASSERT_WITH_HALT(false, "template_match: out of bound");
        return 0;
    }

    //generate square map
    static std::array<uint16_t, 256> q_map{[]{
        std::array<uint16_t, 256> temp = {};
        for(int i = 0; i < 256; ++i) {
            temp[i] = (i * i) >> (lossy_bits * 2);
        }
        return temp;
    }()};

    #define FAST_SQUARE(x) (q_map[std::abs(x)])
    #define FAST_SQRT(x) (integer_sqrt(x))

    int t_mean = int(tmp.mean());
    int s_mean = int(src.mean(Rect2i(offs, tmp.get_size())));

    int64_t num = 0;
    int64_t den_t = 0;
    int64_t den_s = 0;

    for(auto y = 0; y < tmp.get_size().y; y++){
        for(auto x = 0; x < tmp.get_size().x; x++){
            int tmp_val = int(tmp[Vector2i{x,y}]) - t_mean;
            int src_val = int(src[Vector2i{x,y} + offs]) - s_mean;

            num += ((tmp_val * src_val) >> (lossy_bits * 2));
            den_t += FAST_SQUARE(tmp_val);
            den_s += FAST_SQUARE(src_val);
        }
    }

    
    if(num == 0) return 0;
    if(den_t == 0 || den_s == 0) return sign(num);

    real_t ret;

    int64_t den = FAST_SQRT(den_t) * FAST_SQRT(den_s);
    int64_t res = num * 65535 / den;
    s16_to_uni(res, ret);
    return std::abs(ret);

    #undef FAST_SQUARE
    #undef FAST_SQRT
}

}