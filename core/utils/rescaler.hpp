#pragma once

#include "core/stream/ostream.hpp"

namespace ymd{
template<typename D>
struct Rescaler { 
    D scale;
    D offset;

    [[nodiscard]] static constexpr Rescaler from_input_and_output(
        const std::tuple<auto, auto>& input, 
        const std::tuple<auto, auto>& output
    ) {
        const auto [input_start, input_stop] = input;
        const auto [output_start, output_stop] = output;
        
        const D calculated_scale = (static_cast<D>(output_stop) - static_cast<D>(output_start)) / 
                                (static_cast<D>(input_stop) - static_cast<D>(input_start));
        const D calculated_offset = static_cast<D>(output_start) - 
                                   static_cast<D>(input_start) * calculated_scale;
        
        return Rescaler{ 
            .scale = calculated_scale,
            .offset = calculated_offset
        };
    }

    [[nodiscard]] static constexpr Rescaler from_input_and_inverted_output(
        const std::tuple<auto, auto>& input, 
        const std::tuple<auto, auto>& output
    ) {
        const auto [output_start, output_stop] = output;
        return from_input_and_output(input, std::tuple{output_stop, output_start});
    }

    template<typename U>
    [[nodiscard]] static constexpr Rescaler<D> from_scale(const U scale){
        return Rescaler<D>{
            .scale = scale,
            .offset = static_cast<D>(0)
        };
    }

    template<typename U>
    [[nodiscard]] static constexpr Rescaler<D> from_offset(const U offset){
        return Rescaler<D>{
            .scale = static_cast<D>(1),
            .offset = offset
        };
    }

    template<typename U>
    [[nodiscard]] static constexpr Rescaler<D> from_anti_offset(const U offset){
        return Rescaler<D>{
            .scale = static_cast<D>(1),
            .offset = -offset
        };
    }

    template<typename U>
    [[nodiscard]] __fast_inline constexpr Rescaler<D> operator *(const Rescaler<U>& rhs) const {
        auto & lhs = *this;
        return Rescaler<D>{
            .scale = lhs.scale * rhs.scale,
            .offset = lhs.offset * rhs.scale + rhs.offset
        };
    }

    [[nodiscard]] __fast_inline constexpr Rescaler<D> operator +(const Rescaler<D>& rhs) const {
        auto & lhs = *this;
        return Rescaler<D>{
            .scale = lhs.scale + rhs.scale,
            .offset = lhs.offset + rhs.offset
        };
    }

    [[nodiscard]] __fast_inline constexpr Rescaler<D> operator -(const Rescaler<D>& rhs) const {
        auto & lhs = *this;
        return Rescaler<D>{
            .scale = lhs.scale - rhs.scale,
            .offset = lhs.offset - rhs.offset
        };
    }

    template<typename U>
    [[nodiscard]] __fast_inline constexpr Rescaler<D> operator *(const U rhs) const { 
        return Rescaler<D>{
            .scale = scale * rhs,
            .offset = offset * rhs
        };
    }

    [[nodiscard]] __fast_inline constexpr Rescaler<D> operator -() const {
        return Rescaler<D>{
            .scale = -scale,
            .offset = -offset
        };
    }

    [[nodiscard]] __fast_inline constexpr Rescaler<D> inverse() const {
        return Rescaler<D>{
            .scale = 1 / scale,
            .offset = -offset / scale
        };
    }
    
    [[nodiscard]] __fast_inline constexpr D operator()(const D& d) const {
        return scale * d + offset;
    }

    friend OutputStream & operator <<(OutputStream & os, const Rescaler & self){
        return os << os.brackets<'('>()
            << self.scale << os.splitter() 
            << self.offset << os.brackets<')'>();
    }
};
}