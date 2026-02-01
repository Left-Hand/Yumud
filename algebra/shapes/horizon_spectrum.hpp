#pragma once

#include "prelude.hpp"

namespace ymd{

template<typename T, typename D>
struct HorizonSpectrum{
    math::Vec2<T> top_left;
    math::Vec2<T> cell_size;
    T spacing;
    std::span<const D> samples;
    math::Range2<D> sample_range;

    [[nodiscard]] constexpr 
    math::Range2<T> y_range(const Nth nth) const {
        const auto y_stop = top_left.y + cell_size.y;
        // const auto y_top = top_left.y;
        const auto data_height = sample_range.invlerp(samples[nth.count()]) * cell_size.y;
        const auto y_start = y_stop - data_height;
        return math::Range2<T>{y_start, y_stop};
    }

    [[nodiscard]] constexpr 
    math::Rect2<T> bounding_box() const {
        const size_t cnt = samples.size();
        return math::Rect2<T>{top_left, 
            math::Vec2<T>(
                cell_size.x * cnt + 
                static_cast<T>(spacing - 1) * static_cast<size_t>(cnt - 1), 

                cell_size.y
            )
        };
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