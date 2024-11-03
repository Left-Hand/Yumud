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

namespace yumud::nvcv2::Geometry{



PerspectiveConfig perspective_config;
InvPerspectiveConfig inv_perspective_config;


Vector2 perspective(const Vector2 & v){
    auto [x,y] = v;
    real_t inv_s = real_t(1) / (perspective_config.H8*y+real_t(1));
    real_t _x = (perspective_config.H1 * x + perspective_config.H2 * y + perspective_config.H3)*inv_s;
    real_t _y = (perspective_config.H4 * x + perspective_config.H5 * y + perspective_config.H6)*inv_s;
    Vector2 ret = {_x,_y};
    return ret;
}


Vector2 inv_perspective(const Vector2 & v){
    auto [x,y] = v;
    real_t inv_s = real_t(1) / (inv_perspective_config.H8*y+real_t(1));
    real_t _x = (inv_perspective_config.H1 * x + inv_perspective_config.H2 * y + inv_perspective_config.H3)*inv_s;
    real_t _y = (inv_perspective_config.H4 * x + inv_perspective_config.H5 * y + inv_perspective_config.H6)*inv_s;
    Vector2 ret = {_x,_y};
    return ret;
}

Vector2 inv_perspective_fast(const Vector2 & v){
    auto [x,y] = v;
    real_t inv_s = real_t(1) / (inv_perspective_config.H8*y+real_t(1));
    real_t _x = (x + inv_perspective_config.H2 * y)*inv_s;
    real_t _y = (inv_perspective_config.H4 * x + inv_perspective_config.H5 * y)*inv_s;
    Vector2 ret = {_x,_y};

    return ret;
}

void perspective(ImageWritable<Grayscale> & dst,const ImageReadable<Grayscale> & src){
    auto size = dst.get_size();
    for(int _y = 0; _y < size.y; _y++){
        auto [x,y] = inv_perspective_fast({0, _y});
        auto x_step = inv_perspective_fast({1,_y}).x - x;

        for(int _x = 0; _x < size.x; _x++){
            x += x_step;
            if(size.has_point(Vector2i{x,y})){
                dst[{_x,_y}]= src[{x,y}];
            }else{
                dst[{_x,_y}]=0;
            }
        }
    }
}


}