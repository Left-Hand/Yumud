
#pragma once

#include "../nvcv2.hpp"

namespace yumud::nvcv2::Geometry{

struct PerspectiveConfig{
    real_t H1 = real_t(1.0);
    real_t H2 = real_t(3.433333333333334);
    real_t H3 = real_t(0.0);
    real_t H4 = real_t(-7.21363183512792e-17);
    real_t H5 = real_t(3.191489361702128);
    real_t H6 = real_t(-8.30478318108259e-15);
    real_t H7 = real_t(-0.0);
    real_t H8 = real_t(0.036524822695035465);
};

struct InvPerspectiveConfig{
    real_t H1 = real_t(1.00000);
    real_t H2 = real_t(-0.28200);
    real_t H3 = real_t(0.00000);
    real_t H4 = real_t(0.00000);
    real_t H5 = real_t(0.64000);
    real_t H6 = real_t(0.00000);
    real_t H7 = real_t(0.00000);
    real_t H8 = real_t(-0.00600);
};

extern PerspectiveConfig perspective_config;
extern InvPerspectiveConfig inv_perspective_config;

Vector2 perspective(const Vector2 & v);

Vector2 inv_perspective(const Vector2 & v);

Vector2 inv_perspective_fast(const Vector2 & v);

void perspective(ImageWritable<Grayscale> & dst,const ImageReadable<Grayscale> & src);
}