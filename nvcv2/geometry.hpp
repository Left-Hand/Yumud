
#pragma once

#include "nvcv2.hpp"

namespace NVCV2::Geometry{
    struct AffineConfig{
        real_t a = real_t(1.6);
        real_t b = real_t(1.7);
        real_t c = real_t(0);
        real_t d = real_t(0);
        real_t e = real_t(-0.8);
        real_t q = real_t(-1);
    };

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

    AffineConfig affine_config;
    PerspectiveConfig perspective_config;
    InvPerspectiveConfig inv_perspective_config;
    Vector2 affine_pos(const Vector2 & uv){
        auto [a,b,c,d,e, q] = affine_config;
        Vector2 uvo;
        real_t inv_s = 1 / (q * uv.y + 1);
        uvo.x = (uv.x * a + uv.y * b + c) * inv_s;
        uvo.y = (uv.y * d + e) * inv_s;
        return uvo;
    }

    void affine(ImageWritable<Grayscale> & dst, const ImageReadable<Grayscale> & src){
        auto [a,b,c,d,e,q] = affine_config;

        Vector2 uv;
        Vector2 uv_step = dst.uvstep();
        DEBUG_VALUE(uv_step);
        Vector2 uvo;
        uv.y = -1;
        for(auto j = 0; j < dst.get_size().y; j++){
            uv.x = -1;
            



            // real_t x_affine_ratio = a + b * uv.y;
            // uvo.x = uv.x * x_affine_ratio;
            // uvo.y = uv.y * (c + d * uv.y) + e;
            real_t inv_s = 1 / (q * uv.y + 1);
           
            uvo.y = (uv.y * d + e) * inv_s;
            
            // DEBUG_PRINT(j,uv, uvo, uv_step);
            for(auto i = 0; i < dst.get_size().x; i++){
                 uvo.x = (uv.x * a + uv.y * b) * inv_s;
                Vector2 fixed_uv = (uvo + Vector2(1,1));
                Vector2i out_pos = Vector2(fixed_uv.x * (dst.size.x / 2), fixed_uv.y * (dst.size.y / 2));
                // if(i == 0 && j == 0){
                //     DEBUG_PRINT(0, uv, uvo);
                // }
                // if(i == dst.size.x-1 && j == 0){
                //     DEBUG_PRINT(1, uv, uvo);
                // }
                dst[out_pos] = src({i, j});

                uv.x += uv_step.x;
            }
            uv.y += uv_step.y;
            // DEBUG_PRINT(j,uv, uvo, uv_step);
            // real_t uvo_step_x = uv_step.x * a * inv_s;

            // iq_t pixelo_x = src.uv2aero(uvo).x;
            // int pixelo_y = int(src.uv2aero(uvo).y);

            // iq_t pixelo_xstep = uvo_step_x * src.get_size().x / 2;
            // int img_w_minus1 = dst.get_size().x;

            // _iq loop_end_value = iq_t(img_w_minus1).value;

            // _iq pixelo_xq = pixelo_x.value;
            // _iq pixelo_xstepq = pixelo_xstep.value;

            // auto i = 0;
            // auto conv_base = j * dst.get_size().x;

            // while(pixelo_xq < 0){
            //     pixelo_xq += pixelo_xstepq;
            //     dst[{i, j}] = Grayscale::BLACK;
            //     i++;
            // }

            // auto y_base = pixelo_y * src.get_size().x;
            // while(pixelo_xq < loop_end_value){
            //     dst[{i, j}] = src((pixelo_xq >> GLOBAL_Q) + y_base);
            //     pixelo_xq += pixelo_xstepq;
            //     i++;
            // }

            // while(i < src.get_size().x){
            //     dst[{i, j}] = Grayscale::BLACK;
            //     i++;
            // }
            }
        }

    Vector2 perspective(const Vector2 & v){
        auto [x,y] = v;
        // DEBUG_PRINT(v)
        using NVCV2::Geometry::perspective_config;
        real_t inv_s = real_t(1) / (perspective_config.H8*y+real_t(1));
        real_t _x = (perspective_config.H1 * x + perspective_config.H2 * y + perspective_config.H3)*inv_s;
        real_t _y = (perspective_config.H4 * x + perspective_config.H5 * y + perspective_config.H6)*inv_s;
        Vector2 ret = {_x,_y};
        // DEBUG_PRINT(ret);
        return ret;
    }


    Vector2 inv_perspective(const Vector2 & v){
        auto [x,y] = v;
        using NVCV2::Geometry::inv_perspective_config;
        real_t inv_s = real_t(1) / (inv_perspective_config.H8*y+real_t(1));
        real_t _x = (inv_perspective_config.H1 * x + inv_perspective_config.H2 * y + inv_perspective_config.H3)*inv_s;
        real_t _y = (inv_perspective_config.H4 * x + inv_perspective_config.H5 * y + inv_perspective_config.H6)*inv_s;
        Vector2 ret = {_x,_y};
        // DEBUG_PRINT(ret);
        return ret;
    }

    Vector2 inv_perspective_fast(const Vector2 & v){
        using NVCV2::Geometry::inv_perspective_config;

        auto [x,y] = v;
        real_t inv_s = real_t(1) / (inv_perspective_config.H8*y+real_t(1));
        real_t _x = (inv_perspective_config.H1 * x + inv_perspective_config.H2 * y)*inv_s;
        real_t _y = (inv_perspective_config.H4 * x + inv_perspective_config.H5 * y)*inv_s;
        Vector2 ret = {_x,_y};

        return ret;
    }
    
    void perspective(ImageWritable<Grayscale> & dst,const ImageReadable<Grayscale> & src){
    using NVCV2::Geometry::perspective_config;
    auto size = dst.size;
    for(uint8_t _y=0;_y < size.y;_y++){
        auto [x,y] = inv_perspective_fast({0, _y});
        auto x_step = inv_perspective_fast({1,_y}).x - x;

        for(uint8_t _x=0;_x < size.x;_x++){
            x += x_step;
            if(size.has_point(Vector2i{x,y})){
                // dst[{_x,_y}]=Pixels::bilinear_interpol(src, {x,y});
                dst[{_x,_y}]= src[{x,y}];

            }else{
                dst[{_x,_y}]=0;
            }
        }
    }
}
}