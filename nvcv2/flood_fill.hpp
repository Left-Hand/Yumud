#pragma once

#include "nvcv2.hpp"

namespace ymd::nvcv2::Shape{

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

struct BlobFilter{
    Range_t<uint> area_range = {0, UINT_MAX};
    Range_t<uint> width_range = {0, UINT_MAX};
    Range_t<uint> height_range = {0, UINT_MAX};

    static BlobFilter clamp_width(const uint min_width, const uint max_width = UINT_MAX){
        BlobFilter filter;
        filter.width_range = {min_width, max_width};
        return filter;
    }

    static BlobFilter clamp_area(const uint min_area, const uint max_area = UINT_MAX){
        BlobFilter filter;
        filter.area_range = {min_area, max_area};
        return filter;
    }
};


class FloodFill{
// public:
protected:


    using Blobs = sstl::vector<Blob, 16>;
    Blobs m_blobs;

public:


    Image<Grayscale> run(const ImageReadable<Binary> & src, const BlobFilter & filter = BlobFilter());
    auto & blobs() const{return m_blobs;}
};
class SimilarRects{
public:    
    SimilarRects(const real_t _eps) : eps(_eps) {}
    inline bool operator()(const Rect2i& r1, const Rect2i& r2) const{
        real_t delta = eps*(std::min(r1.w, r2.w) + std::min(r1.h, r2.h))*real_t(0.5);
        return std::abs(r1.x - r2.x) <= delta &&
        std::abs(r1.y - r2.y) <= delta &&
        std::abs(r1.x + r1.w - r2.x - r2.w) <= delta &&
        std::abs(r1.y + r1.h - r2.y - r2.h) <= delta;
    }

    real_t eps;
};    


}

__fast_inline ymd::OutputStream & operator<<(ymd::OutputStream & os, const ymd::nvcv2::Shape::Blob & blob){
    using namespace ymd::nvcv2;
    return os << '<' << blob.index << '>' << blob.rect;
}

