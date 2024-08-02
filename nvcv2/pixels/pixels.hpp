#pragma once

#include "../nvcv2.hpp"

namespace NVCV2::Pixels{
    void conv(ImageWritable<RGB565>& dst, const ImageReadable<Grayscale>& src);

    void conv(ImageWritable<RGB565>& dst, const ImageReadable<Binary>& src);

    class UniqueRandomGenerator {
    private:
        static constexpr uint8_t m = 251;
        static constexpr uint8_t a = 37;
        static constexpr uint8_t c = 71;
        std::array<uint8_t, 256> data;

        static constexpr uint8_t iter(const uint8_t x){
            return (a * x + c) % m;
        }
    public:
        UniqueRandomGenerator() {
            // Initialize with unique random numbers using LCG
            std::array<bool, 256> used{};
            used.fill(0);
            // uint8_t x = 0; // Initial seed
            uint8_t x_next = iter(0);
            for (size_t i = 0; i < 256; ++i) {
                // Find next unique number
                uint8_t x_new = x_next;
                // while(used[x_new]){
                    x_new = iter(x_new);
                // }
                x_next = x_new;
                // Mark x as used and assign to data[i]
                used[x_next] = true;
                data[i] = x_next;
            }
            data[0] = 0;
        }

        uint8_t operator[](const uint8_t index) const{
            return data[index];
        }
    };


    void dyeing(ImageWritable<Grayscale>& dst, const ImageReadable<Grayscale>& src);

    auto dyeing(const ImageReadable<Grayscale>& src);

    Grayscale dyeing(const Grayscale in);

    template<typename T>
    requires (std::is_same_v<T, Grayscale> || std::is_same_v<T, Binary>)
    void copy(ImageWritable<T>& dst, const ImageReadable<T>& src) {
        for (auto x = 0; x < MIN(dst.get_size().x, src.get_size().x); x++) {
            for (auto y = 0; y < MIN(dst.get_size().y, src.get_size().y); y++) {
                dst[Vector2i{x, y}] = src[Vector2i{x, y}];
            }
        }
    }

    __inline void fast_diff_opera(Image<Grayscale> & dst, const Image<Grayscale> & src) {
        if((void *)&dst == (void *)&src){
            auto temp = dst.clone();
            fast_diff_opera(temp, src);
            dst = std::move(temp);
            return;
        }

        auto window = dst.get_view().intersection(src.get_view());
        for (auto y = window.y; y < window.y + window.h; y++) {
            for (auto x = window.x; x < window.x + window.w; x++) {
                const int a = src[Vector2i{x,y}];
                const int b = src[Vector2i{x+1,y}];
                const int c = src[Vector2i{x,y+1}];
                dst[{x,y}] = uint8_t(CLAMP(std::max(
                    (ABS(a - c)) * 255 / (a + c),
                    (ABS(a - b) * 255 / (a + b))
                ), 0, 255));
            }
        }
    }

    __inline void fast_bina_opera(Image<Binary> & out,const Image<Grayscale> & em, const uint8_t et,const Image<Grayscale>& dm, const uint8_t dt) {
        const auto size = (Rect2i(Vector2i(), em.get_size()).intersection(Rect2i(Vector2i(), dm.get_size()))).size;
        const auto area = size.x * size.y;

        for (auto i = 0; i < area; i++) {
            out[i] = Binary(((uint8_t)em[i] > et) || ((uint8_t)dm[i] > dt));
        }
    }
    
    void binarization(ImageWritable<Binary>& dst, const ImageReadable<Grayscale>& src, const Grayscale threshold);

    Image<Binary> binarization(const ImageReadable<Grayscale>& src, const Grayscale threshold);
    void ostu(Image<Binary>& dst, const Image<Grayscale>& src);


    void iter_threshold(Image<Binary>& dst, const Image<Grayscale>& src, const real_t & k = 0.5, const real_t & eps = 0.02);

    void max_entropy(const Image<Grayscale>& src,const int thresh);

    int get_huang_fuzzy_threshold(Histogram hist);

    int huang(Image<Binary>& dst, const Image<Grayscale>& src);

    void gamma(Image<Grayscale>& src, const real_t ga);


    template<typename T>
    concept is_monocolour_v = std::same_as<T, Binary> || std::same_as<T, Grayscale>;


    template<is_monocolour_v T>
    void inverse(ImageWithData<T, T>& src) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = uint8_t(~uint8_t(src[i]));
        }
    }


    template<is_monocolour_v T>
    void inverse(ImageWritable<T>& dst, const ImageReadable<T> & src) {
        auto window = dst.get_view().intersection(src.get_view());
        for (auto y = window.y; y < window.y + window.h; y++) {
            for (auto x = window.x; x < window.x + window.w; x++) {
                    dst[{x,y}] = uint8_t(~uint8_t(T(src[{x,y}])));
            }
        }
    }
    template<is_monocolour_v T>
    void and_with(ImageWithData<T, T> & src, ImageWithData<T, T>& op) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = std::min((uint8_t)src[i], (uint8_t)op[i]);
        }
    }

    template<is_monocolour_v T>
    void or_with(ImageWithData<T, T> & src, ImageWithData<T, T>& op) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = std::max((uint8_t)src[i], (uint8_t)op[i]);
        }
    }


    template<is_monocolour_v T>
    void xor_with(ImageWithData<T, T> & src, ImageWithData<T, T>& op) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = ((uint8_t)src[i] ^ (uint8_t)op[i]);
        }
    }


    void mask_with(Image<Grayscale> & src, const ImageReadable<Binary>& op);
    void sum_with(Image<Grayscale> & src, Image<Grayscale>& op);
    void sub_with(Image<Grayscale> & src, Image<Grayscale>& op);
}

