#pragma once

#include "core/stream/ostream.hpp"
#include "types/vectors/Vector2.hpp"

namespace ymd{


template<typename T, typename Policy>
struct CacheOf{

};

template<typename T>
struct BoundingBoxOf{

};

template<typename C>
struct Pixel{
    Vec2u position;
    C color;
};

enum class StrokeAlignment:uint8_t{
    Inside,
    Center,
    Outside,
};

template<typename C>
struct PrimitiveStyle{
    Option<C> fill_color;
    Option<C> stroke_color;
    q16 stroke_width;
    StrokeAlignment stroke_alignment;
};


inline OutputStream & operator<<(OutputStream & os, const StrokeAlignment self){
    switch(self){
        case StrokeAlignment::Inside: return os << "Inside";
        case StrokeAlignment::Center: return os << "Center";
        case StrokeAlignment::Outside: return os << "Outside";
        default: __builtin_unreachable();
    }
}


template<typename C>
inline OutputStream & operator<<(OutputStream & os, const PrimitiveStyle<C> & self){
    const auto _ = os.create_guard();
    os.no_brackets(DISEN);

    return os << os.scoped("Stroke")(os 
        << os.field("fill_color")(os << self.fill_color << os.splitter()) 
        << os.field("stroke_color")(os << self.stroke_color << os.splitter())
        << os.field("stroke_width")(os << self.stroke_width << os.splitter())
        << os.field("stroke_alignment")(os << self.stroke_alignment)
    );
}

}