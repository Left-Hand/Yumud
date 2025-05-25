#pragma once

#include "../nvcv2.hpp"

namespace ymd::nvcv2::pixels{
    void conv(Image<RGB565>& dst, const Image<Grayscale>& src);

    void conv(Image<RGB565>& dst, const Image<Binary>& src);


    void dyeing(Image<Grayscale>& dst, const Image<Grayscale>& src);

    auto dyeing(const Image<Grayscale>& src);

    Grayscale dyeing(const Grayscale in);

    template<typename T>
    requires (std::is_same_v<T, Grayscale> || std::is_same_v<T, Binary>)
    void copy(Image<T>& dst, const Image<T>& src) {
        for (auto x = 0u; x < MIN(dst.size().x, src.size().x); x++) {
            for (auto y = 0u; y < MIN(dst.size().y, src.size().y); y++) {
                dst[Vector2u{x, y}] = src[Vector2u{x, y}];
            }
        }
    }

    __inline void fast_diff_opera(Image<Grayscale> & dst, const Image<Grayscale> & src) {
        if(dst == src){
            auto temp = dst.clone();
            fast_diff_opera(temp, src);
            dst = std::move(temp);
            return;
        }

        auto window_opt = (dst.size().to_rect())
            .intersection(src.size().to_rect());

        if(window_opt.is_none()) return;
        const auto window = window_opt.unwrap();
        for (auto y = window.y(); y < window.y() + window.h(); y++) {
            for (auto x = window.x(); x < window.x() + window.w(); x++) {
                const int a = src[Vector2u{x,y}];
                const int b = src[Vector2u{x+1,y}];
                const int c = src[Vector2u{x,y+1}];
                dst[{x,y}] = uint8_t(CLAMP(std::max(
                    (ABS(a - c)) * 255 / (a + c),
                    (ABS(a - b) * 255 / (a + b))
                ), 0, 255));
            }
        }
    }

    __inline void fast_bina_opera(
            Image<Binary> & out,
            const Image<Grayscale> & em, 
            const uint8_t et,
            const Image<Grayscale>& dm,
            const uint8_t dt) {

        const auto area = Vector2<size_t>{
            MIN(em.size().x, dm.size().x), 
            MIN(em.size().y, dm.size().y)
        }.area();

        for (auto i = 0u; i < area; i++) {
            out[i] = Binary((static_cast<uint8_t>(em[i]) > et) || 
                (static_cast<uint8_t>(dm[i]) > dt));
        }
    }
    
    void binarization(Image<Binary>& dst, const Image<Grayscale>& src, const Grayscale threshold);

    Image<Binary> binarization(const Image<Grayscale>& src, const Grayscale threshold);
    void ostu(Image<Binary>& dst, const Image<Grayscale>& src);


    void iter_threshold(
        Image<Binary>& dst, 
        const Image<Grayscale>& src, 
        const real_t k = real_t(0.5), 
        const real_t eps = real_t(0.02));

    void max_entropy(const Image<Grayscale>& src,const int thresh);

    int get_huang_fuzzy_threshold(Histogram hist);

    int huang(Image<Binary>& dst, const Image<Grayscale>& src);

    uint sum(const Image<Grayscale>& src);

    __inline Grayscale average(const Image<Grayscale>& src){
        return sum(src) / src.size().area();
    }

    void gamma(Image<Grayscale>& src, const real_t ga);


    template<typename T>
    concept is_monocolour_v = std::same_as<T, Binary> || std::same_as<T, Grayscale>;


    template<is_monocolour_v T>
    void inverse(ImageWithData<T, T>& src) {
        for (auto i = 0u; i < src.size().x * src.size().y; i++) {
            src[i] = uint8_t(~uint8_t(src[i]));
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
    void and_with(ImageWithData<T, T> & src, ImageWithData<T, T>& op) {
        for (auto i = 0; i < src.size().x * src.size().y; i++) {
            src[i] = std::min((uint8_t)src[i], (uint8_t)op[i]);
        }
    }

    template<is_monocolour_v T>
    void or_with(ImageWithData<T, T> & src, ImageWithData<T, T>& op) {
        for (auto i = 0; i < src.size().x * src.size().y; i++) {
            src[i] = std::max((uint8_t)src[i], (uint8_t)op[i]);
        }
    }


    template<is_monocolour_v T>
    void xor_with(ImageWithData<T, T> & src, ImageWithData<T, T>& op) {
        for (auto i = 0; i < src.size().x * src.size().y; i++) {
            src[i] = ((uint8_t)src[i] ^ (uint8_t)op[i]);
        }
    }


    void mask_with(Image<Grayscale> & src, const Image<Binary>& op);
    void sum_with(Image<Grayscale> & src, Image<Grayscale>& op);
    void sub_with(Image<Grayscale> & src, Image<Grayscale>& op);
}

