#pragma once

#include "nvcv2.hpp"

namespace NVCV2::Pixels{
    void conv(ImageWritable<RGB565>& dst, const ImageReadable<Grayscale>& src);

    // template<typename ColorType>
    Grayscale bilinear_interpol(const ImageReadable<Grayscale> & img, const Vector2 & pos);

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
        }

        uint8_t operator[](const uint8_t index) const{
            return data[index];
        }
    };


    void dyeing(ImageWritable<Grayscale>& dst, const ImageReadable<Grayscale>& src);

    auto dyeing(const ImageReadable<Grayscale>& src);

    template<typename T>
    requires (std::is_same_v<T, Grayscale> || std::is_same_v<T, Binary>)
    void copy(ImageWritable<T>& dst, const ImageReadable<T>& src) {
        for (auto x = 0; x < MIN(dst.get_size().x, src.get_size().x); x++) {
            for (auto y = 0; y < MIN(dst.get_size().y, src.get_size().y); y++) {
                dst[Vector2i{x, y}] = src(Vector2i{x, y});
            }
        }
    }

    void binarization(ImageWritable<Binary>& dst, const ImageReadable<Grayscale>& src, const Grayscale threshold);

    Image<Binary, Binary> binarization(const ImageReadable<Grayscale>& src, const Grayscale threshold);
    void ostu(Image<Binary, Binary>& dst, const Image<Grayscale, Grayscale>& src);


    void iter_threshold(Image<Binary, Binary>& dst, const Image<Grayscale, Grayscale>& src, const real_t & k = 0.5, const real_t & eps = 0.02);

    void max_entropy(const Image<Grayscale, Grayscale>& src, int thresh);
    int get_huang_fuzzy_threshold(Histogram hist);

    int huang(Image<Binary, Binary>& dst, const Image<Grayscale, Grayscale>& src);

    void gamma(Image<Grayscale, Grayscale>& src, const real_t ga);


    template<typename T>
    concept is_monocolour_v = std::same_as<T, Binary> || std::same_as<T, Grayscale>;


    template<is_monocolour_v T>
    void inverse(Image<T, T>& src) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = uint8_t(~uint8_t(src[i]));
        }
    }


    template<is_monocolour_v T>
    void inverse(ImageWritable<T>& dst, const ImageReadable<T> & src) {
        auto window = dst.get_window().intersection(src.get_window());
        for (auto y = window.y; y < window.y + window.h; y++) {
            for (auto x = window.x; x < window.x + window.w; x++) {
                    dst[{x,y}] = uint8_t(~uint8_t(T(src[{x,y}])));
            }
        }
    }
    template<is_monocolour_v T>
    void and_with(Image<T, T> & src, Image<T, T>& op) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = std::min((uint8_t)src[i], (uint8_t)op[i]);
        }
    }

    template<is_monocolour_v T>
    void or_with(Image<T, T> & src, Image<T, T>& op) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = std::max((uint8_t)src[i], (uint8_t)op[i]);
        }
    }

    // template<is_monocolour_v T>
    __inline void adapative_threshold(ImageWritable<Binary> & dst, const ImageReadable<Grayscale> & src) {
        // if()
        const auto size = (Rect2i(Vector2i(), dst.size).intersection(Rect2i(Vector2i(), src.size))).size;
        // const auto area = size.x * size.y;

        for(int y = 0; y < size.y; y++){
            for(int x = 0; x < size.x; x++){

                uint16_t average=0;
                int i,j;
                for(i=y-3;i<=y+3;i++)
                {
                    for(j=x-3;j<=x+3;j++)
                    {
                        average+=src[{i,j}];
                    }
                }
                average=average/49-23;
                dst[{x,y}] = Binary((((uint8_t)src[{x,y}]) > average) ? Binary(255):Binary(0));
            }
        }
    }
    template<is_monocolour_v T>
    void xor_with(Image<T, T> & src, Image<T, T>& op) {
        for (auto i = 0; i < src.get_size().x * src.get_size().y; i++) {
            src[i] = ((uint8_t)src[i] ^ (uint8_t)op[i]);
        }
    }


    void mask_with(Image<Grayscale, Grayscale> & src, const ImageReadable<Binary>& op);
    void sum_with(Image<Grayscale, Grayscale> & src, Image<Grayscale, Grayscale>& op);
    void sub_with(Image<Grayscale, Grayscale> & src, Image<Grayscale, Grayscale>& op);

    void adaptive_threshold(ImageWritable<Binary> & dst, const ImageReadable<Grayscale> & src);
}
