
#pragma once

#include "nvcv2.hpp"

namespace NVCV2::Geometry{

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
    real_t H1 = real_t(0.9999999999999999);
    real_t H2 = real_t(-1.0757777777777775);
    real_t H3 = real_t(-8.934101195471293e-15);//0
    real_t H4 = real_t(2.2602713083400805e-17);//0
    real_t H5 = real_t(0.3133333333333332);
    real_t H6 = real_t(2.602165396739211e-15);//0
    real_t H7 = real_t(-8.25560087797973e-19);//0
    real_t H8 = real_t(-0.011444444444444443);
};

extern PerspectiveConfig perspective_config;
extern InvPerspectiveConfig inv_perspective_config;

Vector2 perspective(const Vector2 & v);

Vector2 inv_perspective(const Vector2 & v);

Vector2 inv_perspective_fast(const Vector2 & v);

void perspective(ImageWritable<Grayscale> & dst,const ImageReadable<Grayscale> & src);
}