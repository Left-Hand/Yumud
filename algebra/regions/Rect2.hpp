#pragma once

#include "core/platform.hpp"
#include "core/utils/Option.hpp"

#include "algebra/regions/range2.hpp"
#include "algebra/vectors/vec2.hpp"


namespace ymd::math{

template<typename T>
class [[nodiscard]] Rect2{
public:
    using Self = Rect2;
    using Tsigned = std::make_signed_t<T>;

    Vec2<T> top_left;
    Vec2<T> size;

    static constexpr Rect2<T> INF = {
        Vec2<T>{std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest()},
        Vec2<T>{std::numeric_limits<T>::max(), std::numeric_limits<T>::max()}
    };

    static constexpr Rect2<T> ZERO = {
        Vec2<T>{static_cast<T>(0), static_cast<T>(0)},
        Vec2<T>{static_cast<T>(0), static_cast<T>(0)}
    };

    static constexpr Rect2<T> IDENITY = {
        Vec2<T>{static_cast<T>(0), static_cast<T>(0)},
        Vec2<T>{static_cast<T>(1), static_cast<T>(1)}
    };


    [[nodiscard]] __fast_inline constexpr 
    Rect2(
        const std::tuple<T, T, T, T> & _quad
    ):
        top_left(std::get<0>(_quad), std::get<1>(_quad)),
        size(std::get<2>(_quad), std::get<3>(_quad))
    {;}



    template<typename U, typename V = U>
    [[nodiscard]] __fast_inline constexpr 
    Rect2(
        const Vec2<U> & _position,
        const Vec2<V> & _size
    ):
        top_left(static_cast<Vec2<T>>(_position.template floor<T>())), 
        size(static_cast<Vec2<T>>((_position + _size).template ceil<T>() - top_left)){;}

    template<typename U>
    [[nodiscard]] __fast_inline constexpr 
    Rect2(const Rect2<U> & other):
        top_left(static_cast<Vec2<T>>(other.top_left.template floor<T>())), 
        size(static_cast<Vec2<T>>((other.top_left + other.size).template ceil<T>() - top_left)){;}
    [[nodiscard]] __fast_inline static constexpr 
    Rect2 from_uninitialized(){
        return Rect2();
    }

    [[nodiscard]] __fast_inline static constexpr 
    Rect2 from_xywh(
        const T x,
        const T y,
        const T w,
        const T h
    ){
        auto ret = Rect2::from_uninitialized();
        ret.top_left.x = x;
        ret.top_left.y = y;
        ret.size.x = w;
        ret.size.y = h;
        return ret;
    }
    [[nodiscard]] __fast_inline static constexpr 
    Rect2 from_center_and_halfsize(
        const Vec2<T> & center, const Vec2<T> & half_size){
        return Rect2<T>(center - half_size, half_size * 2);
    }

    [[nodiscard]] __fast_inline static constexpr 
    Rect2 from_center_and_size(
        const Vec2<T> & center, const Vec2<T> & size){
        return Rect2<T>(center - size, size);
    }

    template<typename U>
    [[nodiscard]] __fast_inline static constexpr 
    Rect2 from_topleft_and_bottomright(
        const Vec2<T> & a, const Vec2<T> & b){
        return Rect2<T>(a, b-a).abs();
    }

    [[nodiscard]] __fast_inline static constexpr 
    Rect2 from_top_left(
        const Vec2<T> & top_left
    ){
        return Rect2<T>(top_left, Vec2<T>::ZERO);
    }

    template<typename U>
    [[nodiscard]] __fast_inline static constexpr 
    Rect2 from_size(const Vec2<U> _size){
        return Rect2<T>{Vec2<T>::ZERO, _size.template ceil<T>()};
    }

    template<typename Range>
    requires requires(Range r) {
        requires std::ranges::input_range<Range>;
        requires std::same_as<std::ranges::range_value_t<Range>, Vec2<T>>;
    }
    [[nodiscard]] __fast_inline static constexpr 
    Rect2 from_minimal_bounding_box(
            Range&& range){

        auto first = std::ranges::begin(range);
        auto last = std::ranges::end(range);

        if(first == last) return Rect2<T>::ZERO;

        const auto & first_point = *first;

        auto x_min = first_point.x;
        auto x_max = first_point.x;
        auto y_min = first_point.y;
        auto y_max = first_point.y;

        for(auto it = std::next(first); it != last; ++it){
            x_min = MIN(x_min, it->x);
            x_max = MAX(x_max, it->x);

            y_min = MIN(y_min, it->y);
            y_max = MAX(y_max, it->y);
        }

        auto ret = Rect2<T>::from_uninitialized();

        ret.top_left.x = math::floor_cast<T>(x_min);
        ret.top_left.y = math::floor_cast<T>(y_min);

        ret.size.x = math::ceil_cast<T>(x_max) - ret.top_left.x;
        ret.size.y = math::ceil_cast<T>(y_max) - ret.top_left.y;

        return ret;
    }

    [[nodiscard]] __fast_inline static constexpr 
    Rect2 from_minimal_bounding_box(
        const std::initializer_list<Vec2<T>> & points){
            return from_minimal_bounding_box(std::span(points.begin(), points.end()));
        }

    // Add direct component accessors
    [[nodiscard]] __fast_inline constexpr 
    T & x() { return top_left.x; }
    [[nodiscard]] __fast_inline constexpr 
    const T & x() const { return top_left.x; }

    [[nodiscard]] __fast_inline constexpr 
    T & y() { return top_left.y; }
    [[nodiscard]] __fast_inline constexpr 
    const T & y() const { return top_left.y; }

    [[nodiscard]] __fast_inline constexpr 
    T & w() { return size.x; }
    [[nodiscard]] __fast_inline constexpr 
    const T & w() const { return size.x; }

    [[nodiscard]] __fast_inline constexpr 
    T & h() { return size.y; }
    [[nodiscard]] __fast_inline constexpr 
    const T & h() const { return size.y; }

    [[nodiscard]] __fast_inline constexpr 
    bool contains_x(const T p_x) const{
        return p_x >= top_left.x && p_x < top_left.x + size.x;
    }

    [[nodiscard]] __fast_inline constexpr 
    bool contains_y(const T p_y) const{
        return p_y >= top_left.y && p_y < top_left.y + size.y;
    }

    [[nodiscard]] __fast_inline constexpr 
    T area() const {
        return ABS(size.x * size.y);}
    [[nodiscard]] __fast_inline constexpr 
    Vec2<T> center() const {
        return(top_left + size / 2);}
    [[nodiscard]] __fast_inline constexpr 
    Vec2<T> bottom_right() const {
        return(top_left + size);
    }
    
    [[nodiscard]] __fast_inline constexpr 
    Option<Rect2<T>> expand(const T val) const {
        if constexpr(std::is_integral_v<T> and std::is_signed_v<T>)
            if(val < 0) return None;
        return shrink_impl(-static_cast<Tsigned>(val));
    }

    [[nodiscard]] __fast_inline constexpr 
    Option<Rect2<T>> shrink(const T val) const {
        if constexpr(std::is_integral_v<T> and std::is_signed_v<T>)
            if(val < 0) return None;
        return shrink_impl(static_cast<Tsigned>(val));
    }

public:
    
    template<size_t I>
    requires(I < 4)
    [[nodiscard]] __fast_inline constexpr 
    Vec2<T> get_corner() const {
        switch(I){
            case 0: return top_left;
            case 1: return Vec2<T>(top_left.x + size.x, top_left.y);
            case 2: return top_left + size;
            case 3: return Vec2<T>(top_left.x, top_left.y + size.y);
            default: __builtin_unreachable();
        }
    }

    [[nodiscard]] constexpr bool is_regular() const{
        return(this->size.x >= 0 && this->size.y >= 0);
    }

    [[nodiscard]] constexpr Rect2<T> abs() const {
        if(is_regular()) return(*this);
        const T x0 = top_left.x;
        const T x1 = top_left.x + size.x;
        const T y0 = top_left.y;
        const T y1 = top_left.y + size.y;

        return Rect2<T>{
            Vec2<T>{
                static_cast<T>(MIN(x0, x1)), 
                static_cast<T>(MIN(y0, y1)),
            },
            
            Vec2<T>{
                static_cast<T>(ABS(static_cast<Tsigned>(x0) - static_cast<Tsigned>(x1))), 
                static_cast<T>(ABS(static_cast<Tsigned>(y0) - static_cast<Tsigned>(y1)))
            }
        };
    }


    [[nodiscard]] __fast_inline constexpr 
    bool contains_point(const Vec2<T> & point) const {
        return IN_RANGE(point.x, top_left.x, top_left.x + size.x)
            and IN_RANGE(point.y, top_left.y, top_left.y + size.y);
    }

    [[nodiscard]] __fast_inline constexpr 
    bool contains(const Rect2<T> & other) const {
        bool x_ins = this->x_range().contains(other.x_range());
        if(false == x_ins) return false;
        bool y_ins = this->y_range().contains(other.y_range());
        return y_ins;
    }

    [[nodiscard]] __fast_inline constexpr 
    bool is_inside(const Rect2<T> & other) const{
        return other.contains(*this);
    }



    [[nodiscard]] __fast_inline constexpr 
    bool operator==(const Rect2<T> & other){
        return (
            (this->top_left == other.top_left) 
            && (this->size == other.size));
    }

    [[nodiscard]] __fast_inline constexpr 
    bool operator!=(const Rect2<T> & other){
        return (
            (this->top_left != other.top_left )
            || (this->size != other.size));
    }

    [[nodiscard]] __fast_inline constexpr 
    Rect2<T> scale_around_corner(
        const auto & ratio
    ) const{
        Rect2<T> ret = (*this);
        ret.top_left *= ratio;
        ret.size *= ratio;
        return(ret);
    }

    [[nodiscard]] constexpr bool intersects(const Rect2<T> & other) const{
        const auto & self = *this;

        return (self.x() < other.x() + other.w()) &&
                (other.x() < self.x() + self.w()) &&
                (self.y() < other.y() + other.h()) &&
                (other.y() < self.y() + self.h());
    }

    [[nodiscard]] __fast_inline constexpr 
    Rect2<T> shift(
        const Vec2<T> & other
    ) const{
        Rect2<T> ret = (*this);
        ret.top_left += other;
        return ret;
    }

    [[nodiscard]] constexpr Option<Rect2<T>> intersection(const Rect2<T> & other) const{

        const auto ins_position = Vec2<T>(
            MAX(T(this->x()), T(other.x())),
            MAX(T(this->y()), T(other.y()))
        );

        const auto ins_size = Vec2<T>(
            MIN(T(this->x() + this->w()), T(other.x() + other.w())) - ins_position.x,
            MIN(T(this->y() + this->h()), T(other.y() + other.h())) - ins_position.y
        );

        if(ins_size.x < 0 || ins_size.y < 0) return None;
        return Some(Rect2<T>{ins_position, ins_size});
    }

    [[nodiscard]] constexpr Rect2<T> merge(const Rect2<T> & other) const{
        Range2<T> range_x = this->x_range().merge(other.x_range());
        Range2<T> range_y = this->y_range().merge(other.y_range());
        return Rect2<T>(range_x, range_y);
    }

    [[nodiscard]] constexpr Rect2<T> merge(const Vec2<T> & point) const{
        const auto & self = *this;

        auto x_min = math::floor_cast<T>(MIN(self.x(), point.x));
        auto x_max = math::ceil_cast<T>(MAX(self.x() + self.w(), point.x));
        auto y_min = math::floor_cast<T>(MIN(self.y(), point.y));
        auto y_max = math::ceil_cast<T>(MAX(self.y() + self.h(), point.y));

        return Rect2<T>(
            Vec2<T>{x_min, y_min}, 
            Vec2<T>{static_cast<T>(x_max - x_min), static_cast<T>(y_max - y_min)}
        );
    }

    [[nodiscard]] constexpr Vec2<T> constrain(const Vec2<T> & point) const{
        Vec2<T> ret;
        ret.x() = this->x_range().clamp(point.x);
        ret.y() = this->y_range().clamp(point.y);
        return ret;
    }

    [[nodiscard]] constexpr Rect2<T> scale_around_center(const auto & amount)const {
        return Rect2<T>::from_center_and_size(
            this->get_center(), this->size * amount);
    }

    [[nodiscard]] constexpr Range2<T> x_range() const{
        return Range2<T>::from_start_and_length(top_left.x, size.x);
    }

    [[nodiscard]] constexpr Range2<T> y_range() const{
        return Range2<T>::from_start_and_length(top_left.y, size.y);
    }

    [[nodiscard]] constexpr Rect2<T> bounding_box() const{
        return *this;
    }

private:
    [[nodiscard]] constexpr Rect2():
        top_left(Vec2<T>::from_uninitialized()),
        size(Vec2<T>::from_uninitialized())
        {;}

    [[nodiscard]] __fast_inline constexpr 
    Option<Rect2<T>> shrink_impl(
        const Tsigned val
    ) const {
        if constexpr(std::is_integral_v<T>){
            const Tsigned next_pos_x =  static_cast<Tsigned>((top_left.x) + val);
            const Tsigned next_pos_y =  static_cast<Tsigned>((top_left.y) + val);
            const Tsigned next_size_x = static_cast<Tsigned>((size.x) - (val << 1));
            const Tsigned next_size_y = static_cast<Tsigned>((size.y) - (val << 1));
            if(next_pos_x < 0 || next_pos_y < 0) return None;
            if(next_size_x < 0 || next_size_y < 0) return None;
            return Some(Rect2<T>{
                Vec2<T>{
                    static_cast<T>(next_pos_x), 
                    static_cast<T>(next_pos_y)
                },
                Vec2<T>{
                    static_cast<T>(next_size_x), 
                    static_cast<T>(next_size_y)
                }
            });
        }else{
            const T next_size_x = size.x - 2 * val;
            const T next_size_y = size.y - 2 * val;
            if(next_size_x < 0 || next_size_y < 0) return None;
            return Some(Rect2<T>{
                top_left + Vec2<T>{val, val}, 
                Vec2<T>{next_size_x, next_size_y}});
        }
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os    
            << os.field("x")(self.top_left.x) << os.splitter()
            << os.field("y")(self.top_left.y) << os.splitter()
            << os.field("w")(self.size.x) << os.splitter()
            << os.field("h")(self.size.y)
        ;
    }
};

using Rect2i = Rect2<int>;
using Rect2f = Rect2<float>;
using Rect2u = Rect2<uint>;

using Rect2u8 = Rect2<uint8_t>;
using Rect2u16 = Rect2<uint16_t>;

};
