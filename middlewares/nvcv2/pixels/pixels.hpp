#pragma once

#include "middlewares/nvcv2/nvcv2.hpp"

namespace ymd::nvcv2::pixels{
void cast_color(Image<RGB565> & dst, const Image<Gray> & src);

void cast_color(Image<RGB565> & dst, const Image<Binary> & src);


void dyeing(Image<Gray> & dst, const Image<Gray> & src);

auto dyeing(const Image<Gray> & src);

Gray dyeing(const Gray in);


void fast_diff_opera(Image<Gray> & dst, const Image<Gray> & src);
void fast_bina_opera(
    Image<Binary> & out,
    const Image<Gray> & em, 
    const Gray et,
    const Image<Gray> & dm,
    const Gray dt
);

void binarization(Image<Binary> & dst, const Image<Gray> & src, const Gray threshold);

Image<Binary> binarization(const Image<Gray> & src, const Gray threshold);
void ostu(Image<Binary> & dst, const Image<Gray> & src);


void iter_threshold(
    Image<Binary> & dst, 
    const Image<Gray> & src, 
    const real_t k = real_t(0.5), 
    const real_t eps = real_t(0.02));

void max_entropy(const Image<Gray> & src,const int thresh);

int get_huang_fuzzy_threshold(Histogram hist);

int huang(Image<Binary> & dst, const Image<Gray> & src);



void gamma(Image<Gray> & src, const real_t ga);

template<is_monochrome T>
void bitwise_inverse(Image<T> & src) {
    for (size_t i = 0u; i < src.size().x * src.size().y; i++) {
        src.head_ptr()[i] = src.head_ptr()[i].flip();
    }
}


template<is_monochrome T>
void bitwise_inverse(Image<T> & dst, const Image<T> & src) {
    auto window = dst.rect().intersection(src.rect());
    for (auto y = window.y(); y < window.y() + window.h(); y++) {
        for (auto x = window.x(); x < window.x() + window.w(); x++) {
                dst[{x,y}] = uint8_t(~uint8_t(T(src[{x,y}])));
        }
    }
}


template<is_monochrome T>
void bitwise_and(Image<T> & src, Image<T> & op) {
    for (size_t i = 0; i < src.size().x * src.size().y; i++) {
        src[i] = std::min((uint8_t)src[i], (uint8_t)op[i]);
    }
}


template<is_monochrome T>
void bitwise_or(Image<T> & src, Image<T> & op) {
    for (size_t i = 0; i < src.size().x * src.size().y; i++) {
        src[i] = std::max((uint8_t)src[i], (uint8_t)op[i]);
    }
}


template<is_monochrome T>
void bitwise_xor(Image<T> & src, Image<T> & op) {
    for (size_t i = 0; i < src.size().x * src.size().y; i++) {
        src[i] = ((uint8_t)src[i] ^ (uint8_t)op[i]);
    }
}


void bitwise_mask_with(Image<Gray> & src, const Image<Binary> & op);
void bitwise_sum_with(Image<Gray> & src, Image<Gray> & op);
void bitwise_sub_with(Image<Gray> & src, Image<Gray> & op);

constexpr uint64_t sum(const Image<Gray> & image, const math::Rect2u16 & roi){
    uint64_t sum = 0;
    const math::Range2u16 x_range = roi.x_range();
    const math::Range2u16 y_range = roi.y_range();

    for(uint16_t j = y_range.start; j < y_range.stop; ++j){
        const auto * ptr = &(image[math::Vec2u16{x_range.start, j}]);
        for(uint16_t i = 0; i < x_range.length(); ++i){
            sum += ptr[i].to_u8();
        }
    }
    return sum;
}

constexpr uint64_t sum(const Image<Gray> & image){
    return sum(image, math::Rect2u16::from_size(image.size()));
}


constexpr Gray mean(const Image<Gray> & image, const math::Rect2u16 & roi){
    return Gray::from_u8(sum(image, roi) / (roi.area()));
}

constexpr Gray mean(const Image<Gray> & image){
    return mean(image, math::Rect2u16::from_size(image.size()));
}

__inline Gray average(const Image<Gray> & src){
    return Gray::from_u8(pixels::sum(src) / src.size().x_mul_y());
}


__inline math::Vec2u16 uv2pixel(const math::Vec2u16 size, const math::Vec2<iq16> & uv){
    return math::Vec2u16(
        LERP(uint16_t(0), size.x, ((uv.x + 1) / 2)), 
        LERP(uint16_t(0), size.y, (uv.y + 1)/2)
    );
}

__inline math::Vec2<iq16> uv2aero(const math::Vec2u16 size, const math::Vec2<iq16> & uv){
    return math::Vec2<iq16>(((uv.x + 1) * (size.x / 2)), (uv.y + 1) * (size.y / 2));
}

__inline math::Vec2<iq16> pixel2uv(const math::Vec2u16 size,const math::Vec2u16 & pixel){
    return math::Vec2<iq16>(
        INVLERP(size.x / 2, size.x, real_t(pixel.x)), 
        INVLERP(size.y / 2, size.y, real_t(pixel.y)));
}

__inline math::Vec2<iq16> uvstep(const math::Vec2u16 size){
    return math::Vec2<iq16>(real_t(2) / size.x, real_t(2) / size.y);
}
}

