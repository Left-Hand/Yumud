#pragma once

#include "core/stream/ostream.hpp"
#include "types/vectors/Vector2.hpp"
#include "types/regions/Rect2.hpp"
#include "types/colors/rgb/rgb.hpp"

namespace ymd{


template<typename Shape, typename Policy>
struct CacheOf{

};

template<typename Shape>
struct BoundingBoxOf{

};

struct ScanLine{
    Range2<uint16_t> x_range;
    uint16_t y;

    Rect2u16 bounding_box() const{
        return Rect2u16(x_range.start, y, x_range.length(), 1);
    }
};

template<typename Shape>
struct is_placed_t:std::false_type{};

enum class StrokeAlignment:uint8_t{
    Inside,
    Center,
    Outside,
};


struct PrimitiveStyle{
    Option<RGB888> fill_color;
    Option<RGB888> stroke_color;
    q16 stroke_width;
    StrokeAlignment stroke_alignment;
};


template<typename Shape>
requires (is_placed_t<Shape>::value == false)
struct WithPosition final{
    Shape shape;
    Vec2<q16> position;
};



// template<typename Shape>
// requires (is_placed_t<Shape>::value == false)
// struct WithStyle{
//     Shape shape;
//     PrimitiveStyle style;
// };


// template<typename Shape>
// requires (is_placed_t<Shape>::value == true)
// struct WithStyle{
//     Shape shape;
//     PrimitiveStyle style;
// };

template<typename Shape>
struct is_placed_t<WithPosition<Shape>>:std::true_type{};

struct AddPosition{
    Vec2<q16> position;
};

struct AddStyle{
    PrimitiveStyle style;
};

template<typename Shape>
requires (is_placed_t<Shape>::value == false)
constexpr WithPosition<Shape> operator | (const Shape & shape, AddPosition adder){
    return WithPosition<Shape>{shape, adder.position};
}


template<typename Shape>
requires (is_placed_t<Shape>::value == false)
struct BoundingBoxOf<WithPosition<Shape>>{
    [[nodiscard]] __fast_inline static constexpr auto bounding_box(
        const WithPosition<Shape> & shape
    ){
        return BoundingBoxOf<Shape>::bounding_box(shape.object).shift(shape.position);
    }
};


template<typename Shape>
struct ScanLinesIterator{
    //static constexpr from(const Shape & shape);
    //bool has_next() const;
    //Option<ScanLine> next();
};


template<typename Shape>
static constexpr ScanLinesIterator<Shape> make_scanlines_iterator(const Shape & shape){ 
    return ScanLinesIterator<Shape>::from(shape);
}

template<typename Shape>
static constexpr ScanLinesIterator<Shape> make_scanlines_iterator(const Shape & shape, auto y){ 
    return ScanLinesIterator<Shape>::from(shape, y);
}

template<typename Shape>
concept ScanLinesIteratorConcept = 
requires(Shape shape, const Shape& const_shape) {
    { shape.from(const_shape) } -> std::same_as<void>;
    { shape.has_next() } -> std::same_as<bool>;
    { shape.next() } -> std::same_as<Option<ScanLine>>;
};

template<typename T>
concept DrawTargetConcept = requires {
    typename T::Error;
} && requires(T target, const Rect2u16 area) {
    // 只检查方法存在性，不严格检查参数和返回类型
    target.fill_contiguous(area, std::declval<int>()); // 用具体类型测试
    target.fill_solid(area, std::declval<int>());
    
    // 单独检查返回类型（如果可能）
    requires std::same_as<
        decltype(target.fill_contiguous(area, std::declval<int>())),
        Result<void, typename T::Error>
    >;
    requires std::same_as<
        decltype(target.fill_solid(area, std::declval<int>())),
        Result<void, typename T::Error>
    >;
};

template<
    typename Shape
>
requires (is_placed_t<Shape>::value)
struct DrawDispatchIterator final{
    // template<typename Target>
    // Result<void, Error> draw(Target & target) const{
    //     target.draw(shape_, style_);
    // }
};




struct CornerRadii final{
    uint8_t top_left;
    uint8_t top_right;
    uint8_t bottom_right;
    uint8_t bottom_left;
};


template<typename Color>
struct Pixel final{
    Vec2u position;
    Color color;
};



inline OutputStream & operator<<(OutputStream & os, const StrokeAlignment self){
    switch(self){
        case StrokeAlignment::Inside: return os << "Inside";
        case StrokeAlignment::Center: return os << "Center";
        case StrokeAlignment::Outside: return os << "Outside";
        default: __builtin_unreachable();
    }
}


inline OutputStream & operator<<(OutputStream & os, const PrimitiveStyle & self){
    const auto _ = os.create_guard();
    os.no_brackets(DISEN);

    return os << os.scoped("Stroke")(os 
        << os.field("fill_color")(os << self.fill_color << os.splitter()) 
        << os.field("stroke_color")(os << self.stroke_color << os.splitter())
        << os.field("stroke_width")(os << self.stroke_width << os.splitter())
        << os.field("stroke_alignment")(os << self.stroke_alignment)
    );
}

inline OutputStream & operator <<(OutputStream & os, const CornerRadii radii){
    return os << os.scoped("CornerRadii")(os
        << os.field("top_left")(os << radii.top_left << os.splitter())
        << os.field("top_right")(os << radii.top_right << os.splitter())
        << os.field("bottom_right")(os << radii.bottom_right << os.splitter())
        << os.field("bottom_left")(os << radii.bottom_left)
    );
}

}