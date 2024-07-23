#pragma once

#include "../nvcv2.hpp"

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

    ImageWithData<Binary, Binary> binarization(const ImageReadable<Grayscale>& src, const Grayscale threshold);
    void ostu(ImageWithData<Binary, Binary>& dst, const ImageWithData<Grayscale, Grayscale>& src);


    void iter_threshold(ImageWithData<Binary, Binary>& dst, const ImageWithData<Grayscale, Grayscale>& src, const real_t & k = 0.5, const real_t & eps = 0.02);

    void max_entropy(const ImageWithData<Grayscale, Grayscale>& src, int thresh);
    int get_huang_fuzzy_threshold(Histogram hist);

    int huang(ImageWithData<Binary, Binary>& dst, const ImageWithData<Grayscale, Grayscale>& src);

    void gamma(ImageWithData<Grayscale, Grayscale>& src, const real_t ga);


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
        auto window = dst.get_window().intersection(src.get_window());
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


    void mask_with(ImageWithData<Grayscale, Grayscale> & src, const ImageReadable<Binary>& op);
    void sum_with(ImageWithData<Grayscale, Grayscale> & src, ImageWithData<Grayscale, Grayscale>& op);
    void sub_with(ImageWithData<Grayscale, Grayscale> & src, ImageWithData<Grayscale, Grayscale>& op);
}

