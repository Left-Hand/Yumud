#pragma once

#include "sys/core/platform.h"
#include "sys/math/real.hpp"
#include "types/range/range_t.hpp"
#include "types/vector2/vector2_t.hpp"


namespace yumud{

template<arithmetic T>
class Rect2_t{
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

        struct{
            T width;
            T height;
        };
    };

    constexpr Rect2_t():position(Vector2_t<T>()), size(Vector2_t<T>()){;}

    constexpr Rect2_t(const Rect2_t<auto> other):position(static_cast<Vector2_t<T>>(other.position)), size(static_cast<Vector2_t<T>>(other.size)){;}

    constexpr Rect2_t(const Vector2_t<auto> & _position,const Vector2_t<auto> & _size):position(static_cast<Vector2_t<T>>(_position)), size(static_cast<Vector2_t<T>>(_size)){;}


    constexpr Rect2_t(const Vector2_t<auto> & _size):position(), size(_size){;}

    explicit constexpr Rect2_t(const Vector2_t<auto> * points, const size_t cnt){
        if(cnt < 1) return;
        const auto & first_point = points[0];
        Range_t<T> x_range = {first_point.x, first_point.x};
        Range_t<T> y_range = {first_point.y, first_point.y};
        for(size_t i = 1; i < cnt; i++){
            x_range = x_range.merge(points[i].x);
            y_range = y_range.merge(points[i].y);
        }

        *this = Rect2_t<T>(x_range, y_range);
    }

    explicit constexpr Rect2_t(const Range_t<auto> & x_range,const Range_t<auto> & y_range):
            position(Vector2_t<T>(x_range.from, y_range.from)), size(Vector2_t<T>(x_range.length(), y_range.length())){;}

    constexpr Rect2_t(const auto _x,const auto _y,const auto _width,const auto _height):position(Vector2_t<T>(_x,_y)),size(Vector2_t<T>(_width, _height)){;}

    scexpr Rect2_t from_center(const Vector2_t<auto> & center, const Vector2_t<auto> & half_size){
        return Rect2_t<T>(center - half_size, half_size * 2).abs();
    }

    scexpr Rect2_t from_cross(const Vector2_t<auto> & a, const Vector2_t<auto> & b){
        return Rect2_t<T>(a, b-a).abs();
    }

    constexpr T get_area() const {return ABS(size.x * size.y);}
    constexpr Vector2_t<T> get_center() const {return(position + size / 2);}
    constexpr Vector2_t<T> get_position() const {return(position);}
    constexpr Vector2_t<T> get_size() const {return(size);}
    constexpr Vector2_t<T> get_end() const {return(position + size);}

    constexpr bool is_regular() const{
        return(this->size.x >= 0 && this->size.y >= 0);
    }
    constexpr Rect2_t<T> abs() const {
        if(is_regular()) return(*this);
        T x0 = position.x;
        T x1 = position.x + size.x;
        T y0 = position.y;
        T y1 = position.y + size.y;
        return Rect2_t<T>(MIN(x0, x1), MIN(y0, y1), MAX(x0, x1) - MIN(x0, x1), MAX(y0, y1) - MIN(y0, y1));
    }


    constexpr bool has_point(const Vector2_t<auto> & point) const {
        Rect2_t<T> regular = this -> abs();
        bool x_ins = regular.get_x_range().has(point.x);
        if(!x_ins) return false;
        bool y_ins = regular.get_y_range().has(point.y);
        return(y_ins);
    }

    constexpr bool contains(const Rect2_t<auto> & other) const {
        Rect2_t<T> regular = this->abs();
        Rect2_t<T> other_regular = other.abs();
        bool x_ins = regular.get_x_range().contains(other_regular.get_x_range());
        if(false == x_ins) return false;
        bool y_ins = regular.get_y_range().contains(other_regular.get_y_range());
        return y_ins;
    }

    constexpr bool inside(const Rect2_t<auto> & other) const{
        return other.contains(*this);
    }


    constexpr Rect2_t<T> & operator=(const Rect2_t<auto> & other){
        this->position = other.position;
        this->size = other.size;
        return(*this);
    }

    constexpr bool operator==(const Rect2_t<auto> & other){
        return (this->position == other.position && this->size == other.size);
    }

    constexpr bool operator!=(const Rect2_t<auto> & other){
        return (this->position!= other.position || this->size!= other.size);
    }

    constexpr Rect2_t<T> operator+(const Vector2_t<auto> & other) const{
        Rect2_t<T> ret = (*this).abs();
        ret.position += other;
        return(ret);
    }

    constexpr Rect2_t<T> operator*(const arithmetic auto & ratio) const{
        Rect2_t<T> ret = (*this).abs();
        ret.position *= ratio;
        ret.size *= ratio;
        return(*this);
    }

    constexpr Rect2_t<T> operator/(const arithmetic auto & ratio) const{
        Rect2_t<T> ret = (*this).abs();
        ret.position /= ratio;
        ret.size /= ratio;
        return ret;
    }

    [[deprecated]] constexpr Rect2_t<T> & operator-(const Vector2_t<auto> & other){
        Rect2_t<T> ret = (*this).abs();
        ret.position -= other;
        return(*this);
    }

    constexpr bool intersects(const Rect2_t<auto> & other) const{
        Rect2_t<T> regular = this->abs();
        Rect2_t<T> other_regular = other.abs();

        bool x_ins = regular.get_x_range().intersects(other_regular.get_x_range());
        if(!x_ins) return false;
        bool y_ins = regular.get_y_range().intersects(other_regular.get_y_range());
        return y_ins;
    }
    constexpr Rect2_t<T> intersection(const Rect2_t<auto> & other) const{
        Rect2_t<T> regular = this -> abs();
        Rect2_t<T> other_regular = other.abs();

        auto _position = Vector2_t<T>(
            MAX(T(regular.x), T(other_regular.x)),
            MAX(T(regular.y), T(other_regular.y))
        );

        auto _size = Vector2_t<T>(
            MIN(T(regular.x + regular.w), T(other_regular.x + other_regular.w)) - _position.x,
            MIN(T(regular.y + regular.h), T(other_regular.y + other_regular.h)) - _position.y
        );

        if(_size.x < 0 || _size.y < 0) return Rect2_t<T>();
        return Rect2_t<T>(_position, _size);
    }

    constexpr Rect2_t<T> merge(const Rect2_t<auto> & other) const{
        Rect2_t<T> regular = this->abs();
        Rect2_t<T> other_regular = other.abs();
        Range_t<T> range_x = regular.get_x_range().merge(other_regular.get_x_range());
        Range_t<T> range_y = regular.get_y_range().merge(other_regular.get_y_range());
        return Rect2_t<T>(range_x, range_y);
    }

    constexpr Rect2_t<T> merge(const Vector2_t<auto> & point) const{
        Rect2_t<T> regular = this->abs();
        Range_t<T> range_x = regular.get_x_range().merge(point.x);
        Range_t<T> range_y = regular.get_y_range().merge(point.y);
        return Rect2_t<T>(range_x, range_y);
    }

    constexpr auto constrain(const Vector2_t<auto> & point) const{
        Rect2_t<T> regular = this->abs();
        std::remove_cvref_t<decltype(point)> ret;
        ret.x = regular.get_x_range().clamp(point.x);
        ret.y = regular.get_y_range().clamp(point.y);
        return ret;
    }

    constexpr Rect2_t<T> scale(const arithmetic auto & amount)const {
        Rect2_t<T> regular = this->abs();
        Rect2_t<T> ret = Rect2_t<T>(regular.get_center(), regular.size * amount);
        if(ret.is_regular())return ret;
        else return Rect2_t<T>();
    }

    constexpr Rect2_t<T> grow(const arithmetic auto & amount)const {
        Rect2_t<T> regular = this->abs();
        Rect2_t<T> ret = Rect2_t<T>(regular.position - amount * Vector2_t<T>(1,1), regular.size + amount * Vector2_t<T>(2,2));
        if(ret.is_regular())return ret;
        else return Rect2_t<T>();
    }

    constexpr Rect2_t<T> move(const Vector2_t<auto> & offset)const{
        return Rect2_t<T>(position + offset, size);
    }

    constexpr Range_t<T> get_x_range() const{
        return Range_t<T>(position.x, position.x + size.x).abs();
    }

    constexpr Range_t<T> get_y_range() const{
        return Range_t<T>(position.y, position.y + size.y).abs();
    }

    constexpr explicit operator bool() const {
        return (size.x != 0) && (size.y != 0);
    }

    template<arithmetic U>
    constexpr explicit operator U() const {
        return get_area();
    }

    // template<>
    // requires std::is_floating_point_v<T>
    // constexpr explicit operator T() const {
    //     return get_area();
    // }

};

using Rect2i = Rect2_t<int>;
using Window = Rect2i;
using Rect2 = Rect2_t<real_t>;
using Rect2f = Rect2_t<float>;

__fast_inline OutputStream & operator<<(OutputStream & os, const Rect2_t<auto> & value){
    return os << '(' << value.position << ',' << value.size << ')';
}
};
