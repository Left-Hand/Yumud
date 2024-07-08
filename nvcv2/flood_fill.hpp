#pragma once

#include "nvcv2.hpp"

namespace NVCV2::Shape{
class FloodFill{
// public:
protected:
    struct Blob{
        Rect2i rect;
        int area;
        int index;

        constexpr operator int() const {
            return area;
        }

        constexpr operator bool() const {
            return area;
        }
    };

    using Blobs = sstl::vector<Blob, 316>;
    Blobs m_blobs;

    // static void found(ImageWritable<Binary> & map, Blob & blob);
public:

	// Image<Grayscale, Grayscale> run(const ImageReadable<Binary> & src);
    Image<Grayscale, Grayscale> run(const ImageReadable<Binary> & src);
    auto & blobs() const{return m_blobs;}
};
class SimilarRects{
public:    
    SimilarRects(const real_t _eps) : eps(_eps) {}
    inline bool operator()(const Rect2i& r1, const Rect2i& r2) const{
        real_t delta = eps*(std::min(r1.width, r2.width) + std::min(r1.height, r2.height))*real_t(0.5);
        return std::abs(r1.x - r2.x) <= delta &&
        std::abs(r1.y - r2.y) <= delta &&
        std::abs(r1.x + r1.width - r2.x - r2.width) <= delta &&
        std::abs(r1.y + r1.height - r2.y - r2.height) <= delta;
    }

    real_t eps;
};    


}