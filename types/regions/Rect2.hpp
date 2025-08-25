#pragma once

#include "core/platform.hpp"
#include "core/utils/Option.hpp"

#include "types/regions/range2.hpp"
#include "types/vectors/vector2.hpp"


namespace ymd{

template<arithmetic T>
class [[nodiscard]] Rect2{
public:
    using Tsigned = std::make_signed_t<T>;

    Vec2<T> position;
    Vec2<T> size;

    static constexpr Rect2<T> INF = {
        Vec2<T>{std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest()},
        Vec2<T>{std::numeric_limits<T>::max(), std::numeric_limits<T>::max()}
    };

    [[nodiscard]] __fast_inline constexpr Rect2():
        position(Vec2<T>(static_cast<T>(0),static_cast<T>(0))), 
        size(Vec2<T>(static_cast<T>(0),static_cast<T>(0))){;}

    [[nodiscard]] constexpr Rect2(
        const T _x,const T _y,const T _width,const T _height
    ):
        position(Vec2<T>(_x,_y)),
        size(Vec2<T>(_width, _height)){;}

    template<typename U>
    [[nodiscard]] __fast_inline constexpr Rect2(const Rect2<U> & other):
        position(static_cast<Vec2<T>>(other.position)), 
        size(static_cast<Vec2<T>>(other.size)){;}

    constexpr Rect2<T> & operator=(const Rect2<T> & other){
        this->position = other.position;
        this->size = other.size;
        return (*this);
    }

    [[nodiscard]] __fast_inline constexpr Rect2(
        const Vec2<T> & _position,
        const Vec2<T> & _size
    ):
        position(static_cast<Vec2<T>>(_position)), 
        size(static_cast<Vec2<T>>(_size)){;}

    [[nodiscard]] __fast_inline constexpr Rect2(
        const Range2<T> & x_range,
        const Range2<T> & y_range
    ):
        position(Vec2<T>(x_range.from, y_range.from)), 
        size(Vec2<T>(x_range.length(), y_range.length())){;}


    [[nodiscard]] __fast_inline static constexpr Rect2 from_center_and_halfsize(
        const Vec2<T> & center, const Vec2<T> & half_size){
        return Rect2<T>(center - half_size, half_size * 2);
    }

    [[nodiscard]] __fast_inline static constexpr Rect2 from_center_and_size(
        const Vec2<T> & center, const Vec2<T> & size){
        return Rect2<T>(center - size, size);
    }

    [[nodiscard]] __fast_inline static constexpr Rect2 from_corners(
        const Vec2<T> & a, const Vec2<T> & b){
        return Rect2<T>(a, b-a).abs();
    }

    [[nodiscard]] __fast_inline static constexpr Rect2 from_top_left(
        const Vec2<T> & top_left
    ){
        return Rect2<T>(top_left, Vec2<T>::ZERO);
    }

    [[nodiscard]] __fast_inline static constexpr Rect2 from_size(const Vec2<T> _size){
        return Rect2<T>({0,0}, _size);
    }

    template<typename Range>
    requires requires(Range r) {
        requires std::ranges::input_range<Range>;
        requires std::same_as<std::ranges::range_value_t<Range>, Vec2<T>>;
    }
    [[nodiscard]] __fast_inline static constexpr Rect2 from_minimal_bounding_box(
            Range&& range){

        auto first = std::ranges::begin(range);
        auto last = std::ranges::end(range);

        if(first == last) return Rect2<T>();

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

        return from_corners({x_min, y_min}, {x_max, y_max});
    }

    [[nodiscard]] __fast_inline static constexpr Rect2 from_minimal_bounding_box(
        const std::initializer_list<Vec2<T>> & points){
            return from_minimal_bounding_box(std::span(points.begin(), points.end()));
        }

    // Add direct component accessors
    [[nodiscard]] __fast_inline constexpr T & x() { return position.x; }
    [[nodiscard]] __fast_inline constexpr const T & x() const { return position.x; }

    [[nodiscard]] __fast_inline constexpr T & y() { return position.y; }
    [[nodiscard]] __fast_inline constexpr const T & y() const { return position.y; }

    [[nodiscard]] __fast_inline constexpr T & w() { return size.x; }
    [[nodiscard]] __fast_inline constexpr const T & w() const { return size.x; }

    [[nodiscard]] __fast_inline constexpr T & h() { return size.y; }
    [[nodiscard]] __fast_inline constexpr const T & h() const { return size.y; }

    [[nodiscard]] __fast_inline constexpr bool has_x(const T p_x) const{
        return p_x >= position.x && p_x < position.x + size.x;
    }

    [[nodiscard]] __fast_inline constexpr bool has_y(const T p_y) const{
        return p_y >= position.y && p_y < position.y + size.y;
    }

    [[nodiscard]] __fast_inline constexpr T get_area() const {
        return ABS(size.x * size.y);}
    [[nodiscard]] __fast_inline constexpr Vec2<T> center() const {
        return(position + size / 2);}
    [[nodiscard]] __fast_inline constexpr Vec2<T> bottom_right() const {
        return(position + size);
    }

    [[nodiscard]] __fast_inline constexpr Vec2<T> top_left() const {
        return(position);
    }
    
    [[nodiscard]] __fast_inline constexpr Option<Rect2<T>> expand(const T val) const {
        if constexpr(std::is_integral_v<T> and std::is_signed_v<T>)
            if(val < 0) return None;
        return shrink_impl(-static_cast<Tsigned>(val));
    }

    [[nodiscard]] __fast_inline constexpr Option<Rect2<T>> shrink(const T val) const {
        if constexpr(std::is_integral_v<T> and std::is_signed_v<T>)
            if(val < 0) return None;
        return shrink_impl(static_cast<Tsigned>(val));
    }
private:
    [[nodiscard]] __fast_inline constexpr Option<Rect2<T>> shrink_impl(
        const Tsigned val
    ) const {
        if constexpr(std::is_integral_v<T>){
            const Tsigned new_size_x = Tsigned(size.x) - 2 * Tsigned(val);
            const Tsigned new_size_y = Tsigned(size.y) - 2 * Tsigned(val);
            if(new_size_x < 0 || new_size_y < 0) return None;
            return Some(Rect2<T>{
                position + Vec2<T>{val, val}, 
                Vec2<T>{
                    static_cast<T>(new_size_x), 
                    static_cast<T>(new_size_y)}});
        }else{
            const T new_size_x = size.x - 2 * val;
            const T new_size_y = size.y - 2 * val;
            if(new_size_x < 0 || new_size_y < 0) return None;
            return Some(Rect2<T>{
                position + Vec2<T>{val, val}, 
                Vec2<T>{new_size_x, new_size_y}});
        }
    }
public:
    
    template<size_t I>
    requires(I < 4)
    [[nodiscard]] __fast_inline constexpr Vec2<T> get_corner() const {
        switch(I){
            case 0: return position;
            case 1: return Vec2<T>(position.x + size.x, position.y);
            case 2: return position + size;
            case 3: return Vec2<T>(position.x, position.y + size.y);
            default: __builtin_unreachable();
        }
    }

    [[nodiscard]] constexpr bool is_regular() const{
        return(this->size.x >= 0 && this->size.y >= 0);
    }

    [[nodiscard]] constexpr Rect2<T> abs() const {
        if(is_regular()) return(*this);
        const T x0 = position.x;
        const T x1 = position.x + size.x;
        const T y0 = position.y;
        const T y1 = position.y + size.y;

        return Rect2<T>{
            MIN(x0, x1), 
            MIN(y0, y1), 
            static_cast<T>(ABS(static_cast<Tsigned>(x0) - static_cast<Tsigned>(x1))), 
            static_cast<T>(ABS(static_cast<Tsigned>(y0) - static_cast<Tsigned>(y1)))
        };
    }


    [[nodiscard]] __fast_inline constexpr bool has_point(const Vec2<T> & point) const {
        return IN_RANGE(point.x, position.x, position.x + size.x)
            and IN_RANGE(point.y, position.y, position.y + size.y);
    }

    [[nodiscard]] __fast_inline constexpr bool contains(const Rect2<T> & other) const {
        bool x_ins = this->get_x_range().contains(other.get_x_range());
        if(false == x_ins) return false;
        bool y_ins = this->get_y_range().contains(other.get_y_range());
        return y_ins;
    }

    [[nodiscard]] __fast_inline constexpr bool is_inside(const Rect2<T> & other) const{
        return other.contains(*this);
    }



    [[nodiscard]] __fast_inline constexpr bool operator==(const Rect2<T> & other){
        return (
            (this->position == other.position) 
            && (this->size == other.size));
    }

    [[nodiscard]] __fast_inline constexpr bool operator!=(const Rect2<T> & other){
        return (
            (this->position != other.position )
            || (this->size != other.size));
    }

    [[nodiscard]] __fast_inline constexpr Rect2<T> scale_around_corner(
        const arithmetic auto & ratio
    ) const{
        Rect2<T> ret = (*this);
        ret.position *= ratio;
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

    [[nodiscard]] __fast_inline constexpr Rect2<T> shift(
        const Vec2<T> & other
    ) const{
        Rect2<T> ret = (*this);
        ret.position += other;
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
        Range2<T> range_x = this->get_x_range().merge(other.get_x_range());
        Range2<T> range_y = this->get_y_range().merge(other.get_y_range());
        return Rect2<T>(range_x, range_y);
    }

    [[nodiscard]] constexpr Rect2<T> merge(const Vec2<T> & point) const{
        const auto & self = *this;

        auto x_min = MIN(self.x(), point.x);
        auto x_max = MAX(self.x() + self.w(), point.x);
        auto y_min = MIN(self.y(), point.y);
        auto y_max = MAX(self.y() + self.h(), point.y);

        return Rect2<T>(x_min, y_min, x_max - x_min, y_max - y_min);
    }

    [[nodiscard]] constexpr Vec2<T> constrain(const Vec2<T> & point) const{
        Vec2<T> ret;
        ret.x() = this->get_x_range().clamp(point.x);
        ret.y() = this->get_y_range().clamp(point.y);
        return ret;
    }

    [[nodiscard]] constexpr Rect2<T> scale_around_center(const arithmetic auto & amount)const {
        return Rect2<T>::from_center_and_size(
            this->get_center(), this->size * amount);
    }

    [[nodiscard]] constexpr Range2<T> get_x_range() const{
        return Range2<T>::from_start_and_length(position.x, size.x);
    }

    [[nodiscard]] constexpr Range2<T> get_y_range() const{
        return Range2<T>::from_start_and_length(position.y, size.y);
    }
};

using Rect2i = Rect2<int>;
using Rect2f = Rect2<float>;
using Rect2u = Rect2<uint>;

using Rect2u8 = Rect2<uint8_t>;
using Rect2u16 = Rect2<uint16_t>;

template<typename T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Rect2<T> & value){
    return os << os.brackets<'('>() 
        << value.position << os.splitter() << 
        value.size << os.brackets<')'>();
}
};
