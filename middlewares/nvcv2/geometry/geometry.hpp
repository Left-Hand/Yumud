
#pragma once

#include "nvcv2.hpp"

namespace ymd::nvcv2::geometry{

struct [[nodiscard]] PerspectiveConfig{
    iq16 H1; 
    iq16 H2; 
    iq16 H3; 
    iq16 H4;
    iq16 H5; 
    iq16 H6;
    iq16 H7;
    iq16 H8;
};

struct [[nodiscard]] InvPerspectiveConfig{
    iq16 H1; 
    iq16 H2; 
    iq16 H3; 
    iq16 H4;
    iq16 H5; 
    iq16 H6;
    iq16 H7;
    iq16 H8;
};

Vec2<iq16> perspective(const Vec2<iq16> & v);

Vec2<iq16> inv_perspective(const Vec2<iq16> & v);

Vec2<iq16> inv_perspective_fast(const Vec2<iq16> & v);

void perspective(
    Image<Gray> & dst,
    const Image<Gray> & src);
}