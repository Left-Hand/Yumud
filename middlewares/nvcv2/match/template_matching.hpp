#pragma once

#include "middlewares/nvcv2/nvcv2.hpp"

namespace ymd::nvcv2::match{

iq16 template_match(
    const Image<Binary> & src, 
    const Image<Binary> & tmp, 
    const math::Vec2u & offset);

iq16 template_match(
    const Image<Gray> & src, 
    const Image<Gray> & tmp, 
    const math::Vec2u & offset);


template<is_monochrome T>
__inline iq16 template_match(const Image<T> & src, const Image<T> & tmp){
    return template_match(src, tmp, {0,0});
}
}