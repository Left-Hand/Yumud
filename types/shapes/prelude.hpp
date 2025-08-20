#pragma once

#include "core/stream/ostream.hpp"
#include "types/vectors/Vector2.hpp"

namespace ymd{


template<typename Obj, typename Policy>
struct CacheOf{

};

template<typename Obj>
struct BoundingBoxOf{

};

template<typename T>
struct ScanLine{
    Range2<T> x_range;
    T y;
};

template<typename Obj>
struct is_placed_t:std::false_type{};


template<typename Obj, typename T>
requires (is_placed_t<Obj>::value == false)
struct WithPosition final{
    const Obj & object;
    Vec2<T> position;
};


template<typename Obj, typename T>
struct is_placed_t<WithPosition<Obj, T>>:std::true_type{};

template<typename Obj, typename T>
requires (is_placed_t<Obj>::value == false)
struct BoundingBoxOf<WithPosition<Obj, T>>{
    [[nodiscard]] __fast_inline static constexpr auto to_bounding_box(
        const WithPosition<Obj, T> & shape
    ){
        return BoundingBoxOf<Obj>::to_bounding_box(shape.object).shift(shape.position);
    }
};


template<
    typename Target, 
    typename Shape
>
requires (is_placed_t<Shape>::value)
struct DrawDispatcher final{
    //
};

struct CornerRadii final{
    uint8_t top_left;
    uint8_t top_right;
    uint8_t bottom_right;
    uint8_t bottom_left;
};


template<typename C>
struct Pixel final{
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