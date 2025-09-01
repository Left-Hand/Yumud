#pragma once

#include "prelude.hpp"

namespace ymd{

template<typename T, typename D>
struct HorizonSpectrum{
    Vec2<T> top_left;
    Vec2<T> cell_size;

    std::span<const D> samples;
    Range2<D> sample_range;

    constexpr Range2<T> y_range(const Nth nth) const {
        const auto y_stop = top_left.y + cell_size.y;
        // const auto y_top = top_left.y;
        const auto data_height = sample_range.invlerp(samples[nth.count()]) * cell_size.y;
        const auto y_start = y_stop - data_height;
        return Range2<T>{y_start, y_stop};
    }

    constexpr Rect2<T> bounding_box() const {
        return Rect2<T>{top_left, Vec2<T>(cell_size.x * samples.size(), cell_size.y)};
    }

    friend OutputStream & operator << (OutputStream & os, const HorizonSpectrum & self){
        return os << os.brackets<'('>() << 
            self.top_left << os.splitter() << 
            self.cell_size << os.splitter() <<
            self.samples << os.splitter() <<
            self.sample_range << os.brackets<')'>()
            ;
    }
};

template<typename T, typename D>
struct is_placed_t<HorizonSpectrum<T, D>>:std::true_type {};


}