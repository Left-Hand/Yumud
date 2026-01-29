#include "geometry.hpp"

// [[-1.84703558e-01 -5.87262594e-01  1.57500039e+02]
//  [-3.87374789e-03 -4.95839731e-01  9.95243309e+01]
//  [-1.09015779e-04 -5.47825447e-03  1.00000000e+00]]
// [[ 6.19560053e-01 -3.34241736e+00  2.26000000e+02]       
//  [ 1.31478812e-02 -2.08711834e+00  2.01000000e+02]       
//  [ 3.87561003e-05 -1.17335352e-02  1.00000000e+00]]  


// #define H1 2.045*(10^-1)
// #define H2 -1.320*(10^0)
// #define H3 7.577*(10^1)
// #define H4 0.000*(10^0)
// #define H5 1.232*(10^-1)
// #define H6 0.000*(10^0)
// #define H7 0.000*(10^-18)
// #define H8 -1.391*(10^-2)
// #define H9 1.000*(10^0)

namespace ymd::nvcv2::geometry{


static constexpr PerspectiveConfig perspective_config{
    .H1 = iq16(1.0),
    .H2 = iq16(3.433333333333334),
    .H3 = iq16(0.0),
    .H4 = iq16(-7.21363183512792e-17),
    .H5 = iq16(3.191489361702128),
    .H6 = iq16(-8.30478318108259e-15),
    .H7 = iq16(-0.0),
    .H8 = iq16(0.036524822695035465),
};
static constexpr InvPerspectiveConfig inv_perspective_config{
    .H1 = iq16(1.00000),
    .H2 = iq16(-0.28200),
    .H3 = iq16(0.00000),
    .H4 = iq16(0.00000),
    .H5 = iq16(0.64000),
    .H6 = iq16(0.00000),
    .H7 = iq16(0.00000),
    .H8 = iq16(-0.00600),
};

math::Vec2<iq16> perspective(const math::Vec2<iq16> & v){
    auto [x,y] = v;
    iq16 inv_s = iq16(1) / (perspective_config.H8*y+iq16(1));
    iq16 _x = (perspective_config.H1 * x + perspective_config.H2 * y + perspective_config.H3)*inv_s;
    iq16 _y = (perspective_config.H4 * x + perspective_config.H5 * y + perspective_config.H6)*inv_s;
    math::Vec2<iq16> ret = {_x,_y};
    return ret;
}


math::Vec2<iq16> inv_perspective(const math::Vec2<iq16> & v){
    auto [x,y] = v;
    iq16 inv_s = iq16(1) / (inv_perspective_config.H8*y+iq16(1));
    iq16 _x = (inv_perspective_config.H1 * x + inv_perspective_config.H2 * y + inv_perspective_config.H3)*inv_s;
    iq16 _y = (inv_perspective_config.H4 * x + inv_perspective_config.H5 * y + inv_perspective_config.H6)*inv_s;
    math::Vec2<iq16> ret = {_x,_y};
    return ret;
}

math::Vec2<iq16> inv_perspective_fast(const math::Vec2<iq16> & v){
    auto [x,y] = v;
    iq16 inv_s = iq16(1) / (inv_perspective_config.H8*y+iq16(1));
    iq16 _x = (x + inv_perspective_config.H2 * y)*inv_s;
    iq16 _y = (inv_perspective_config.H4 * x + inv_perspective_config.H5 * y)*inv_s;
    math::Vec2<iq16> ret = {_x,_y};

    return ret;
}

void perspective(Image<Gray> & dst,const Image<Gray> & src){
    auto size = static_cast<const Image<Gray> &>(dst).size();
    for(size_t _y = 0; _y < size.y; _y++){
        auto [x,y] = inv_perspective_fast({0, _y});
        auto x_step = inv_perspective_fast({1,_y}).x - x;

        for(size_t _x = 0; _x < size.x; _x++){
            x += x_step;
            if(size.has_point(math::Vec2u{size_t(x),size_t(y)})){
                dst[{_x,_y}]= src[{size_t(x),size_t(y)}];
            }else{
                dst[{_x,_y}] = Gray::black();
            }
        }
    }
}


}