#pragma once

#include "../nvcv2.hpp"

namespace ymd::nvcv2::pixels{
    void conv(Image<RGB565>& dst, const Image<Gray>& src);

    void conv(Image<RGB565>& dst, const Image<Binary>& src);


    void dyeing(Image<Gray>& dst, const Image<Gray>& src);

    auto dyeing(const Image<Gray>& src);

    Gray dyeing(const Gray in);

    template<typename T>
    requires (std::is_same_v<T, Gray> || std::is_same_v<T, Binary>)
    void copy(Image<T>& dst, const Image<T>& src) {
        for (auto x = 0u; x < MIN(dst.size().x, src.size().x); x++) {
            for (auto y = 0u; y < MIN(dst.size().y, src.size().y); y++) {
                dst[Vec2u{x, y}] = src[Vec2u{x, y}];
            }
        }
    }

    __inline void fast_diff_opera(Image<Gray> & dst, const Image<Gray> & src) {
        if(dst.is_shared_with(src)){
            auto temp = dst.clone();
            fast_diff_opera(temp, src);
            dst = std::move(temp);
            return;
        }

        auto may_window = (Rect2u::from_size(dst.size()))
            .intersection(Rect2u::from_size(src.size()));

        if(may_window.is_none()) return;
        const auto window = may_window.unwrap();
        for (auto y = window.y(); y < window.y() + window.h(); y++) {
            for (auto x = window.x(); x < window.x() + window.w(); x++) {
                const uint8_t a = src[Vec2u{x,y}].to_u8();
                const uint8_t b = src[Vec2u{x+1,y}].to_u8();
                const uint8_t c = src[Vec2u{x,y+1}].to_u8();
                dst[{x,y}] = Gray::from_u8(uint8_t(CLAMP(std::max(
                    (ABS(a - c)) * 255 / (a + c),
                    (ABS(a - b) * 255 / (a + b))
                ), 0, 255)));
            }
        }
    }

    __inline void fast_bina_opera(
            Image<Binary> & out,
            const Image<Gray> & em, 
            const Gray et,
            const Image<Gray>& dm,
            const Gray dt) {

        const auto area = Vec2<size_t>{
            MIN(em.size().x, dm.size().x), 
            MIN(em.size().y, dm.size().y)
        }.x_mul_y();

        for (size_t i = 0u; i < area; i++) {
            const auto o = Binary(em[i].to_binary(et)).and_with(Binary(dm[i].to_binary(dt)));
            out[i] = o;
        }
    }
    
    void binarization(Image<Binary>& dst, const Image<Gray>& src, const Gray threshold);

    Image<Binary> binarization(const Image<Gray>& src, const Gray threshold);
    void ostu(Image<Binary>& dst, const Image<Gray>& src);


    void iter_threshold(
        Image<Binary>& dst, 
        const Image<Gray>& src, 
        const real_t k = real_t(0.5), 
        const real_t eps = real_t(0.02));

    void max_entropy(const Image<Gray>& src,const int thresh);

    int get_huang_fuzzy_threshold(Histogram hist);

    int huang(Image<Binary>& dst, const Image<Gray>& src);



    void gamma(Image<Gray>& src, const real_t ga);


    template<typename T>
    concept is_monocolour_v = std::same_as<T, Binary> || std::same_as<T, Gray>;


    template<is_monocolour_v T>
    void inverse(Image<T>& src) {
        for (auto i = 0u; i < src.size().x * src.size().y; i++) {
            src[i] = src[i].flip();
        }
    }


    template<is_monocolour_v T>
    void inverse(Image<T>& dst, const Image<T> & src) {
        auto window = dst.rect().intersection(src.rect());
        for (auto y = window.y(); y < window.y() + window.h(); y++) {
            for (auto x = window.x(); x < window.x() + window.w(); x++) {
                    dst[{x,y}] = uint8_t(~uint8_t(T(src[{x,y}])));
            }
        }
    }
    template<is_monocolour_v T>
    void and_with(Image<T> & src, Image<T>& op) {
        for (auto i = 0; i < src.size().x * src.size().y; i++) {
            src[i] = std::min((uint8_t)src[i], (uint8_t)op[i]);
        }
    }

    template<is_monocolour_v T>
    void or_with(Image<T> & src, Image<T>& op) {
        for (auto i = 0; i < src.size().x * src.size().y; i++) {
            src[i] = std::max((uint8_t)src[i], (uint8_t)op[i]);
        }
    }


    template<is_monocolour_v T>
    void xor_with(Image<T> & src, Image<T>& op) {
        for (auto i = 0; i < src.size().x * src.size().y; i++) {
            src[i] = ((uint8_t)src[i] ^ (uint8_t)op[i]);
        }
    }


    void mask_with(Image<Gray> & src, const Image<Binary>& op);
    void sum_with(Image<Gray> & src, Image<Gray>& op);
    void sub_with(Image<Gray> & src, Image<Gray>& op);

    
    // constexpr ColorType mean(const Rect2u & view) const;
    // constexpr ColorType mean() const{return mean(this->size().to_rect());}

    // constexpr uint64_t sum(const Rect2u & roi) const;
    // constexpr uint64_t sum() const{return sum(this->size().to_rect());}
    // constexpr ColorType bilinear_interpol(const Vec2<iq16> & pos) const;



    constexpr uint64_t sum(const Image<Gray> & image, const Rect2u & roi){
        uint64_t sum = 0;
        const Range2u x_range = roi.x_range();
        const Range2u y_range = roi.y_range();

        for(uint j = y_range.start; j < y_range.stop; ++j){
            const auto * ptr = &(image[Vec2u{x_range.start, j}]);
            for(uint i = 0; i < x_range.length(); ++i){
                sum += ptr[i].to_u8();
            }
        }
        return sum;
    }

    constexpr uint64_t sum(const Image<Gray> & image){
        return sum(image, Rect2u::from_size(image.size()));
    }


    constexpr Gray mean(const Image<Gray> & image, const Rect2u & roi){
        return Gray::from_u8(sum(image, roi) / (roi.area()));
    }

    constexpr Gray mean(const Image<Gray> & image){
        return mean(image, Rect2u::from_size(image.size()));
    }

    __inline Gray average(const Image<Gray>& src){
        return Gray::from_u8(pixels::sum(src) / src.size().x_mul_y());
    }


// template<typename ColorType>
// constexpr ColorType Image<ColorType>::bilinear_interpol(const Vec2<iq16> & pos) const {
//     Vec2u pos_i = {uint(pos.x), uint(pos.y)};
//     // return img(pos_i);
//     if(!this->size().has_point(pos_i) || !this->size().has_point(pos_i + Vec2u{1,1})) return ColorType();
//     Vec2<iq16> pos_frac = {frac(pos.x), frac(pos.y)};
    
//     const auto & self = *this;
//     if(pos_frac.x){
//         // uint16_t x_u16;
//         // uni_to_u16(pos_frac.x, x_u16);
//         // uint8_t color_up = x_u16 * uint8_t(img(pos_i)) >> 16;
//         // color_up += ((~x_u16) * uint8_t(img(pos_i + Vec2u(1, 0))) >> 16);

//         // return ColorType(color_up);
//         // if(!pos_frac.y){
//         //     return color_up >> 16;
//         //     // return img(pos_i);
//         // }else{
//         //     uint32_t color_dn = (uint16_t)x_u16 * (uint8_t)img(pos_i + Vec2u(0, 1)) + (~x_u16) * img(pos_i + Vec2u(1, 1));
//         //     uint16_t y_u16;
//         //     uni_to_u16(pos_frac.y, y_u16);
//         //     return ((color_up >> 16) * y_u16 + (color_dn >> 16) * (~y_u16)) >> 16;
//         // }

//                     // uint16_t x_u16;
//         // uni_to_u16(pos_frac.x, x_u16);
//         // int c1 =  int(img(pos_i));
//         // int c2 = int(img(pos_i + Vec2u(1, 0)));
//         // return int((real_t(1)-pos_frac.x) * c1 + pos_frac.x * c2);
//         int color_up = int(LERP(int(self[pos_i]), int(self[pos_i + Vec2u(1, 0)]), pos_frac.x));
//         // return color_up;
//         if(!pos_frac.y){
//             return color_up;
//         }else{
//             // uint32_t color_dn = x_u16 * img(pos_i + Vec2u(0, 1)) + (~x_u16) * img(pos_i + Vec2u(1, 1));
//             int color_dn = int(LERP(int(self[pos_i + Vec2u(0, 1)]), int(self[pos_i + Vec2u(1, 1)]), pos_frac.x));
//             // uint16_t y_u16;
//             // uni_to_u16(pos_frac.y, y_u16);
//             // return ((color_up >> 16) * y_u16 + (color_dn >> 16) * (~y_u16)) >> 16;
//             return int(LERP(color_up, color_dn, pos_frac.y));
//         }
//     }else{
//         // if(pos_frac.y){
//         //     // uint16_t y_u16;
//         //     // uni_to_u16(pos_frac.y, y_u16);
//         //     // return (y_u16 * img(pos_i) + (~y_u16) * img(pos_i + Vec2u(0, 1))) >> 16;
//         //     return LERP(pos_frac.y, img(pos_i), img(pos_i + Vec2u(0, 1)));
//         // }else{
//             return self[pos_i];
//         // }
//     }
//     // return (ColorType)LERP(
//     //         pos_frac.y,
//     //         LERP(pos_frac.x, operator()(pos_i), operator()(pos_i + Vec2u(1, 0))),
//     //         LERP(pos_frac.x, operator()(pos_i + Vec2u(0, 1)), operator()(pos_i + Vec2u(1, 1))));
// }

    __inline Vec2u uv2pixel(const Vec2u size, const Vec2<iq16> & uv){
        return Vec2u(
            uint(LERP(0u, size.x, ((uv.x + 1) / 2))), 
            uint(LERP(0u, size.y, (uv.y + 1)/2)));
    }

    __inline Vec2<iq16> uv2aero(const Vec2u size, const Vec2<iq16> & uv){
        return Vec2<iq16>(((uv.x + 1) * (size.x / 2)), (uv.y + 1) * (size.y / 2));
    }

    __inline Vec2<iq16> pixel2uv(const Vec2u size,const Vec2u & pixel){
        return Vec2<iq16>(
            INVLERP(size.x / 2, size.x, real_t(pixel.x)), 
            INVLERP(size.y / 2, size.y, real_t(pixel.y)));
    }

    __inline Vec2<iq16> uvstep(const Vec2u size){
        return Vec2<iq16>(real_t(2) / size.x, real_t(2) / size.y);
    }
}

