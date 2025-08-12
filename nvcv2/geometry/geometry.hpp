
#pragma once

#include "../nvcv2.hpp"

namespace ymd::nvcv2::geometry{

struct PerspectiveConfig{
    real_t H1; 
    real_t H2; 
    real_t H3; 
    real_t H4;
    real_t H5; 
    real_t H6;
    real_t H7;
    real_t H8;
};

struct InvPerspectiveConfig{
    real_t H1; 
    real_t H2; 
    real_t H3; 
    real_t H4;
    real_t H5; 
    real_t H6;
    real_t H7;
    real_t H8;
};

Vec2<q16> perspective(const Vec2<q16> & v);

Vec2<q16> inv_perspective(const Vec2<q16> & v);

Vec2<q16> inv_perspective_fast(const Vec2<q16> & v);

void perspective(
    __restrict Image<Gray> & dst,
    __restrict const Image<Gray> & src);
}