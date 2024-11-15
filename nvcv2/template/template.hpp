#pragma once

#include "../nvcv2.hpp"

namespace ymd::nvcv2::Match{

real_t template_match(const Image<Binary> &, const Image<Binary> &, const Vector2i &);
real_t template_match(const Image<Grayscale> &, const Image<Grayscale> &, const Vector2i &);


template<is_monochrome T>
__inline real_t template_match(const Image<T> & src, const Image<T> & tmp){
    return template_match(src, tmp, {0,0});
}
}