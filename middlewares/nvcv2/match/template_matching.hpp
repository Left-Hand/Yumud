#pragma once

#include "middlewares/nvcv2/nvcv2.hpp"

namespace ymd::nvcv2::match{

real_t template_match(
    const Image<Binary> & src, 
    const Image<Binary> & tmp, 
    const Vec2u & offset);

real_t template_match(
    const Image<Gray> & src, 
    const Image<Gray> & tmp, 
    const Vec2u & offset);


template<is_monochrome T>
__inline real_t template_match(const Image<T> & src, const Image<T> & tmp){
    return template_match(src, tmp, {0,0});
}
}