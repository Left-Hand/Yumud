#pragma once

#include "core/platform.hpp"

#include "types/range/range.hpp"
#include "types/vector2/vector2.hpp"


namespace ymd{

template<arithmetic T>
class [[nodiscard]] Rect2_t{
public:

    union{
        Vector2_t<T> position;
        struct{
            T x;
            T y;
        };
    };

    union{
        Vector2_t<T> size;
        struct{
            T w;
            T h;
        };
    };

    static constexpr Rect2_t<T> INF = Rect2_t<T>(
        std::numeric_limits<T>::min(), 
        std::numeric_limits<T>::min(),
        std::numeric_limits<T>::max(), 
        std::numeric_limits<T>::max()
    );

    [[nodiscard]] constexpr Rect2_t():position(Vector2_t<T>(0,0)), size(Vector2_t<T>(0,0)){}

    [[nodiscard]] constexpr Rect2_t(const Rect2_t<T> & other):
        position(static_cast<Vector2_t<T>>(other.position)), 
        size(static_cast<Vector2_t<T>>(other.size)){}

    [[nodiscard]] constexpr Rect2_t(const Vector2_t<T> & _position,const Vector2_t<T> & _size):
        position(static_cast<Vector2_t<T>>(_position)), 
        size(static_cast<Vector2_t<T>>(_size)){}

    [[nodiscard]] constexpr Rect2_t(const Range2_t<T> & x_range,const Range2_t<T> & y_range):
            position(Vector2_t<T>(x_range.from, y_range.from)), 
            size(Vector2_t<T>(x_range.length(), y_range.length())){;}

    [[nodiscard]] constexpr Rect2_t(const T _x,const T _y,const T _width,const T _height):position(Vector2_t<T>(_x,_y)),size(Vector2_t<T>(_width, _height)){;}

    [[nodiscard]] static constexpr Rect2_t from_center_and_halfsize(
        const Vector2_t<T> & center, const Vector2_t<T> & half_size){
        return Rect2_t<T>(center - half_size, half_size * 2);
    }

    [[nodiscard]] static constexpr Rect2_t from_center_and_size(
        const Vector2_t<T> & center, const Vector2_t<T> & size){
        return Rect2_t<T>(center - size, size);
    }

    [[nodiscard]] static constexpr Rect2_t from_corners(const Vector2_t<T> & a, const Vector2_t<T> & b){
        return Rect2_t<T>(a, b-a).abs();
    }

    
    [[nodiscard]] static constexpr Rect2_t from_size(const Vector2_t<T> _size){
        return Rect2_t<T>({0,0}, _size);
    }

    [[nodiscard]] static constexpr Rect2_t from_minimal_bounding_box(
            const std::span<const Vector2_t<T>> points){
        if(points.size() == 0) while(true);
        const auto & first_point = points[0];
        auto x_min = first_point.x;
        auto x_max = first_point.x;
        auto y_min = first_point.y;
        auto y_max = first_point.y;

        for(size_t i = 1; i < points.size(); i++){
            x_min = MIN(x_min, points[i].x);
            x_max = MAX(x_max, points[i].x);

            y_min = MIN(y_min, points[i].y);
            y_max = MAX(y_max, points[i].y);
        }

        // return Rect2_t<T>(x_min, y_min, 
        //     x_max - x_min, y_max - y_min);
    
        return from_corners({x_min, y_min}, {x_max, y_max});
    }

    [[nodiscard]] static constexpr Rect2_t from_minimal_bounding_box(
        const std::initializer_list<Vector2_t<T>> & points){
            return from_minimal_bounding_box(std::span(points.begin(), points.end()));
        }

    [[nodiscard]] constexpr T get_area() const {return ABS(size.x * size.y);}
    [[nodiscard]] constexpr Vector2_t<T> get_center() const {return(position + size / 2);}
    [[nodiscard]] constexpr Vector2_t<T> get_end() const {return(position + size);}
    
    [[nodiscard]] constexpr Rect2_t<T> expand(const T val) const {
        return {position - Vector2_t<T>{val, val}, size + Vector2_t<T>{2 * val, 2 * val}};
    }

    [[nodiscard]] constexpr Rect2_t<T> shrink(const T val) const {
        return {position + Vector2_t<T>{val, val}, size - Vector2_t<T>{2 * val, 2 * val}};
    }
    
    [[nodiscard]] constexpr Vector2_t<T> get_corner(const size_t idx) const {
        switch(idx % 4){
            default:
            case 0:return position;
            case 1:return Vector2_t<T>(position.x + size.x, position.y);
            case 2:return position + size;
            case 3:return Vector2_t<T>(position.x, position.y + size.y);
        }
    }

    constexpr bool is_regular() const{
        return(this->size.x >= 0 && this->size.y >= 0);
    }

    constexpr Rect2_t<T> abs() const {
        if(is_regular()) return(*this);
        T x0 = position.x;
        T x1 = position.x + size.x;
        T y0 = position.y;
        T y1 = position.y + size.y;
        return Rect2_t<T>(MIN(x0, x1), MIN(y0, y1), ABS(x0 - x1), ABS(y0 - y1));
    }


    constexpr bool has_point(const Vector2_t<T> & point) const {
        // bool x_ins = this->get_x_range().has(point.x);
        // if(!x_ins) return false;
        // bool y_ins = this->get_y_range().has(point.y);
        // return(y_ins);
        return IN_RANGE(point.x, position.x, position.x + size.x)
            and IN_RANGE(point.y, position.y, position.y + size.y);
    }

    constexpr bool contains(const Rect2_t<T> & other) const {
        bool x_ins = this->get_x_range().contains(other.get_x_range());
        if(false == x_ins) return false;
        bool y_ins = this->get_y_range().contains(other.get_y_range());
        return y_ins;
    }

    constexpr bool inside(const Rect2_t<T> & other) const{
        return other.contains(*this);
    }


    constexpr Rect2_t<T> & operator=(const Rect2_t<T> & other){
        this->position = other.position;
        this->size = other.size;
        return (*this);
    }

    constexpr bool operator==(const Rect2_t<T> & other){
        return (this->position == other.position && this->size == other.size);
    }

    constexpr bool operator!=(const Rect2_t<T> & other){
        return (this->position!= other.position || this->size!= other.size);
    }

    constexpr Rect2_t<T> operator+(const Vector2_t<T> & other) const{
        Rect2_t<T> ret = (*this);
        ret.position += other;
        return(ret);
    }

    constexpr Rect2_t<T> operator*(const arithmetic auto & ratio) const{
        Rect2_t<T> ret = (*this);
        ret.position *= ratio;
        ret.size *= ratio;
        return(ret);
    }

    constexpr Rect2_t<T> operator/(const arithmetic auto & ratio) const{
        Rect2_t<T> ret = (*this);
        ret.position /= ratio;
        ret.size /= ratio;
        return ret;
    }

    [[nodiscard]] constexpr bool intersects(const Rect2_t<T> & other) const{
        // return this->get_x_range().intersects(other.get_x_range())
        //     and this->get_y_range().intersects(other.get_y_range());
        const auto & self = *this;

        return (self.x < other.x + other.w) &&
                (other.x < self.x + self.w) &&
                (self.y < other.y + other.h) &&
                (other.y < self.y + self.h);
        // return (ABS(other.position.x - this->position.x) * 2) < (other.size.x + this->size.x)
        // and (ABS(other.position.y - this->position.y) * 2) < (other.size.y + this->size.y);
    }

    [[nodiscard]] constexpr Rect2_t<T> intersection(const Rect2_t<T> & other) const{

        auto _position = Vector2_t<T>(
            MAX(T(this->x), T(other.x)),
            MAX(T(this->y), T(other.y))
        );

        auto _size = Vector2_t<T>(
            MIN(T(this->x + this->w), T(other.x + other.w)) - _position.x,
            MIN(T(this->y + this->h), T(other.y + other.h)) - _position.y
        );

        if(_size.x < 0 || _size.y < 0) return Rect2_t<T>();
        return Rect2_t<T>(_position, _size);
    }

    [[nodiscard]] constexpr Rect2_t<T> merge(const Rect2_t<T> & other) const{
        Range2_t<T> range_x = this->get_x_range().merge(other.get_x_range());
        Range2_t<T> range_y = this->get_y_range().merge(other.get_y_range());
        return Rect2_t<T>(range_x, range_y);
    }

    [[nodiscard]] constexpr Rect2_t<T> merge(const Vector2_t<T> & point) const{
        Range2_t<T> range_x = this->get_x_range().merge(point.x);
        Range2_t<T> range_y = this->get_y_range().merge(point.y);
        return Rect2_t<T>(range_x, range_y);
    }

    [[nodiscard]] constexpr auto constrain(const Vector2_t<T> & point) const{
        std::remove_cvref_t<decltype(point)> ret;
        ret.x = this->get_x_range().clamp(point.x);
        ret.y = this->get_y_range().clamp(point.y);
        return ret;
    }

    [[nodiscard]] constexpr Rect2_t<T> scale(const arithmetic auto & amount)const {
        Rect2_t<T> ret = Rect2_t<T>(this->get_center(), this->size * amount);
        if(ret.is_regular())return ret;
        else return Rect2_t<T>();
    }

    [[nodiscard]] constexpr Rect2_t<T> grow(const arithmetic auto & amount)const {
        Rect2_t<T> ret = Rect2_t<T>(
            this->position - amount * Vector2_t<T>(1,1), 
            this->size + amount * Vector2_t<T>(2,2)
        );
        if(ret.is_regular())return ret;
        else return Rect2_t<T>();
    }

    [[nodiscard]] constexpr Rect2_t<T> shift(const Vector2_t<T> & offset)const{
        return Rect2_t<T>(position + offset, size);
    }

    [[nodiscard]] constexpr Range2_t<T> get_x_range() const{
        return Range2_t<T>::from_start_and_length(position.x, size.x);
    }

    [[nodiscard]] constexpr Range2_t<T> get_y_range() const{
        return Range2_t<T>::from_start_and_length(position.y, size.y);
    }
};

using Rect2i = Rect2_t<int>;
using Rect2u = Rect2_t<uint>;
using Rect2 = Rect2_t<real_t>;
using Rect2f = Rect2_t<float>;

template<typename T>
__fast_inline OutputStream & operator<<(OutputStream & os, const Rect2_t<T> & value){
    return os << os.brackets<'('>() 
        << value.position << os.splitter() << 
        value.size << os.brackets<')'>();
}
};
