#ifndef __RECT2_HPP__

#define __RECT2_HPP__

#include "../sys/core/platform.h"
#include "../types/real.hpp"
#include "../types/range/range_t.hpp"
#include "../types/vector2/vector2_t.hpp"

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

    __fast_inline constexpr Rect2_t(){;}

    template<arithmetic U>
    __fast_inline constexpr Rect2_t(const Rect2_t<U> other):position(other.position), size(other.size){;}

    template<arithmetic U>
    __fast_inline constexpr Rect2_t(const Vector2_t<U> & _position,const Vector2_t<U> & _size):position(_position), size(_size){;}

    template<arithmetic U>
    __fast_inline constexpr Rect2_t(const Range_t<U> & x_range,const Range_t<U> & y_range):
            position(Vector2_t<T>(x_range.from, y_range.from)), size(Vector2_t<T>(x_range.length(), y_range.length())){;}
    template<arithmetic U>
    __fast_inline constexpr Rect2_t(U x, U y, U width, U height):position(Vector2_t<U>(x,y)),size(Vector2_t<U>(width, height)){;}

    template<arithmetic U>
    static constexpr Rect2_t from_center(const Vector2_t<U> & center, const Vector2_t<U> & size){
        return Rect2_t<T>(center - size, size * 2);
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
        bool x_ins = regular.get_x_range().has_value(point.x);
        if(!x_ins) return false;
        bool y_ins = regular.get_y_range().has_value(point.y);
        return(y_ins);
    }

    constexpr bool contains(const Rect2_t<auto> & other) const {
        Rect2_t<T> regular = this->abs();
        Rect2_t<T> other_regular = other.abs();
        bool x_ins = regular.get_x_range().contains(other_regular.get_x_range());
        if(!x_ins)return false;
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

    constexpr Rect2_t<T> & operator+(const Vector2_t<auto> & other){
        this->position += other;
        return(*this);
    }

    [[deprecated]] constexpr Rect2_t<T> & operator-(const Vector2_t<auto> & other){
        this->position -= other;
        return(*this);
    }

    template<arithmetic U>
    constexpr bool intersects(const Rect2_t<U> other) const{
        Rect2_t<T> regular = this->abs();
        Rect2_t<T> other_regular = other.abs();

        bool x_ins = regular.get_x_range().intersects(other_regular.get_x_range());
        if(!x_ins) return false;
        bool y_ins = regular.get_y_range().intersects(other_regular.get_y_range());
        return y_ins;
    }


    template<arithmetic U>
    constexpr Rect2_t<T> intersection(const Rect2_t<U> & other) const{
        Rect2_t<T> regular = this -> abs();
        Rect2_t<T> other_regular = other.abs();
        Rangei range_x = regular.get_x_range().intersection(other_regular.get_x_range());
        if (!bool(range_x)) return Rect2_t<T>();
        Rangei range_y = regular.get_y_range().intersection(other_regular.get_y_range());
        if (!bool(range_y)) return Rect2_t<T>();

        return Rect2_t<T>(range_x,range_y);
    }

    template<arithmetic U>
    constexpr Rect2_t<T> merge(const Rect2_t<U> & other) const{
        Rect2_t<T> regular = this->abs();
        Rect2_t<T> other_regular = other.abs();
        Rangei range_x = regular.get_x_range().merge(other_regular.get_x_range());
        // Rangei range_x = regular.get_x_range();
        Rangei range_y = regular.get_y_range();
        // .merge(other_regular.get_y_range());
        return Rect2_t<T>(range_x, range_y);
    }

    template<arithmetic U>
    constexpr Rect2_t<T> merge(const Vector2_t<U> & point) const{
        Rect2_t<T> regular = this->abs();
        Rangei range_x = regular.get_x_range().merge(point.x);
        Rangei range_y = regular.get_y_range().merge(point.y);
        return Rect2_t<T>(range_x, range_y);
    }

    template<arithmetic U>
    constexpr Vector2_t<U> constrain(const Vector2_t<U> & point) const{
        Rect2_t<T> regular = this->abs();
        Vector2_t<U> ret;
        ret.x = regular.get_x_range().clamp(point.x);
        ret.y = regular.get_y_range().clamp(point.y);
        return ret;
    }

    template<arithmetic U>
    constexpr Rect2_t<T> scale(const U amount)const {
        Rect2_t<T> regular = this->abs();
        Rect2_t<T> ret = Rect2_t<T>(regular.get_center(), regular.size * amount);
        if(ret.is_regular())return ret;
        else return Rect2_t<T>();
    }

    template<arithmetic U>
    constexpr Rect2_t<T> grow(const U amount)const {
        Rect2_t<T> regular = this->abs();
        Rect2_t<T> ret = Rect2_t<T>(regular.position - amount * Vector2_t<T>(1,1), regular.size + amount * Vector2_t<T>(2,2));
        if(ret.is_regular())return ret;
        else return Rect2_t<T>();
    }

    constexpr Rect2_t<T> move(const Vector2_t<auto> offset)const{
        return Rect2_t<T>(position + offset, size);
    }

    constexpr Range_t<T> get_x_range() const{
        return Range_t<T>(position.x, position.x + size.x);
    }

    constexpr Range_t<T> get_y_range() const{
        return Range_t<T>(position.y, position.y + size.y);
    }

    constexpr explicit operator bool() const {
        return (size.x != 0) && (size.y != 0);
    }

    constexpr explicit operator T() const {
        return get_area();
    }

    constexpr  __no_inline explicit operator String() const{
        return toString();
    }

    __no_inline String toString(unsigned char decimalPlaces = 2) const {
        return ('(' + String(position) + ',' + String(size) + ')');
    }
};

typedef Rect2_t<int> Rect2i;
#endif