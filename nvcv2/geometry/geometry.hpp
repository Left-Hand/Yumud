
#pragma once

#include "../nvcv2.hpp"

namespace ymd::nvcv2::Geometry{

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

Vector2 perspective(const Vector2 & v);

Vector2 inv_perspective(const Vector2 & v);

Vector2 inv_perspective_fast(const Vector2 & v);

void perspective(
    __restrict ImageWritable<Grayscale> & dst,
    __restrict const ImageReadable<Grayscale> & src);
}