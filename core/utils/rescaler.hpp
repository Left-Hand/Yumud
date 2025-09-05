#pragma once

#include "core/stream/ostream.hpp"

namespace ymd{
template<typename D>
struct Rescaler { 
    D scale;
    D offset;

    static constexpr Rescaler from_input_and_output(
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

    static constexpr Rescaler from_input_and_inverted_output(
        const std::tuple<auto, auto>& input, 
        const std::tuple<auto, auto>& output
    ) {
        const auto [output_start, output_stop] = output;
        return from_input_and_output(input, std::tuple{output_stop, output_start});
    }
    
    __fast_inline constexpr D operator()(const D& d) const {
        return scale * d + offset;
    }

    friend OutputStream & operator <<(OutputStream & os, const Rescaler & self){
        return os << os.brackets<'('>()
            << self.scale << os.splitter() 
            << self.offset << os.brackets<')'>();
    }
};
}