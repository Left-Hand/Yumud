#pragma once

#include "../nvcv2.hpp"

namespace ymd::nvcv2::match{

real_t template_match(
    __restrict const Image<Binary> & src, 
    __restrict const Image<Binary> & tmp, 
    const Vector2u & offset);

real_t template_match(
    __restrict const Image<Grayscale> & src, 
    __restrict const Image<Grayscale> & tmp, 
    const Vector2u & offset);


template<is_monochrome T>
__inline real_t template_match(const Image<T> & src, const Image<T> & tmp){
    return template_match(src, tmp, {0,0});
}
}