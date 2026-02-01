#pragma once

#include "middlewares/nvcv2/nvcv2.hpp"

namespace ymd::nvcv2::pixels{

template<typename ColorType>
constexpr ColorType Image<ColorType>::bilinear_interpol(const math::Vec2<iq16> & pos) const {
    math::Vec2u16 pos_i = {size_t(pos.x), size_t(pos.y)};
    // return img(pos_i);
    if(!this->size().has_point(pos_i) || !this->size().has_point(pos_i + math::Vec2u16{1,1})) return ColorType();
    math::Vec2<iq16> pos_frac = {frac(pos.x), frac(pos.y)};

    const auto & self = *this;
    if(pos_frac.x){
        // uint16_t x_u16;
        // uni_to_u16(pos_frac.x, x_u16);
        // uint8_t color_up = x_u16 * uint8_t(img(pos_i)) >> 16;
        // color_up += ((~x_u16) * uint8_t(img(pos_i + math::Vec2u16(1, 0))) >> 16);

        // return ColorType(color_up);
        // if(!pos_frac.y){
        //     return color_up >> 16;
        //     // return img(pos_i);
        // }else{
        //     uint32_t color_dn = (uint16_t)x_u16 * (uint8_t)img(pos_i + math::Vec2u16(0, 1)) + (~x_u16) * img(pos_i + math::Vec2u16(1, 1));
        //     uint16_t y_u16;
        //     uni_to_u16(pos_frac.y, y_u16);
        //     return ((color_up >> 16) * y_u16 + (color_dn >> 16) * (~y_u16)) >> 16;
        // }

                    // uint16_t x_u16;
        // uni_to_u16(pos_frac.x, x_u16);
        // int c1 =  int(img(pos_i));
        // int c2 = int(img(pos_i + math::Vec2u16(1, 0)));
        // return int((real_t(1)-pos_frac.x) * c1 + pos_frac.x * c2);
        int color_up = int(LERP(int(self[pos_i]), int(self[pos_i + math::Vec2u16(1, 0)]), pos_frac.x));
        // return color_up;
        if(!pos_frac.y){
            return color_up;
        }else{
            // uint32_t color_dn = x_u16 * img(pos_i + math::Vec2u16(0, 1)) + (~x_u16) * img(pos_i + math::Vec2u16(1, 1));
            int color_dn = int(LERP(int(self[pos_i + math::Vec2u16(0, 1)]), int(self[pos_i + math::Vec2u16(1, 1)]), pos_frac.x));
            // uint16_t y_u16;
            // uni_to_u16(pos_frac.y, y_u16);
            // return ((color_up >> 16) * y_u16 + (color_dn >> 16) * (~y_u16)) >> 16;
            return int(LERP(color_up, color_dn, pos_frac.y));
        }
    }else{
        // if(pos_frac.y){
        //     // uint16_t y_u16;
        //     // uni_to_u16(pos_frac.y, y_u16);
        //     // return (y_u16 * img(pos_i) + (~y_u16) * img(pos_i + math::Vec2u16(0, 1))) >> 16;
        //     return LERP(pos_frac.y, img(pos_i), img(pos_i + math::Vec2u16(0, 1)));
        // }else{
            return self[pos_i];
        // }
    }
    // return (ColorType)LERP(
    //         pos_frac.y,
    //         LERP(pos_frac.x, operator()(pos_i), operator()(pos_i + math::Vec2u16(1, 0))),
    //         LERP(pos_frac.x, operator()(pos_i + math::Vec2u16(0, 1)), operator()(pos_i + math::Vec2u16(1, 1))));
}


}