#pragma once

#include "nvcv2/nvcv2.hpp"

namespace ymd::nvcv2::Shape{

struct Blob{
    Rect2u rect;
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
    Range2<uint32_t> area_range =     Range2<uint32_t>{uint32_t(0), UINT32_MAX};
    Range2<uint32_t> width_range =    Range2<uint32_t>{uint32_t(0), UINT32_MAX};
    Range2<uint32_t> height_range =   Range2<uint32_t>{uint32_t(0), UINT32_MAX};

    // static BlobFilter clamp_width(const uint32_t min_width, const uint32_t max_width = UINT32_MAX){
    //     BlobFilter filter;
    //     filter.w()idth_range = {min_width, max_width};
    //     return filter;
    // }

    // static constexpr BlobFilter clamp_area(const uint32_t min_area, const uint32_t max_area = UINT32_MAX){
    //     BlobFilter filter;
    //     filter.area_range = {min_area, max_area};
    //     return filter;
    // }

    constexpr bool is_valid(const Blob & blob) const{ 
        return true;
    }
};


class FloodFill{
// public:
protected:
    using Blobs = sstl::vector<Blob, 16>;
    Blobs m_blobs;

public:
    Image<Gray> run(const Image<Binary> & src, const BlobFilter & filter = BlobFilter());
    auto & blobs() const{return m_blobs;}
};

class SimilarRects{
public:    
    SimilarRects(const real_t _eps) : eps(_eps) {}
    inline bool operator()(const Rect2u& r1, const Rect2u& r2) const{
        real_t delta = eps*(std::min(r1.w(), r2.w()) + std::min(r1.h(), r2.h()))*real_t(0.5);
        return ABS(r1.x() - r2.x()) <= delta &&
            ABS(r1.y() - r2.y()) <= delta &&
            ABS(r1.x() + r1.w() - r2.x() - r2.w()) <= delta &&
            ABS(r1.y() + r1.h() - r2.y() - r2.h()) <= delta;
    }

    real_t eps;
};    


}

__fast_inline ymd::OutputStream & operator<<(ymd::OutputStream & os, const ymd::nvcv2::Shape::Blob & blob){
    using namespace ymd::nvcv2;
    return os << '<' << blob.index << '>' << blob.rect;
}

