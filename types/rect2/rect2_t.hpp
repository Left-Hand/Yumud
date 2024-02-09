#ifndef __RECT2_HPP__

#define __RECT2_HPP__

#include "../real.hpp"
#include "../range/range_t.hpp"
#include "../vector2/vector2_t.hpp"
#include "../../src/defines/comm_inc.h"

template<typename T>
class Rect2_t{
public:
    Vector2_t<T> position = Vector2_t<T>(0,0);
    Vector2_t<T> size = Vector2_t<T>(0,0);

    Rect2_t(){;}

    template<typename U>
    Rect2_t(const Rect2_t<U> other):position(other.position), size(other.size){;}

    template<typename U>
    Rect2_t(const Vector2_t<U> & _position,const Vector2_t<U> & _size):position(_position), size(_size){;}

    template<typename U>
    Rect2_t(U x, U y, U width, U height):position(Vector2_t<U>(x,y)),size(Vector2_t<U>(width, height)){;}

    T get_area() const {return ABS(size.x * size.y);}
    Vector2_t<T> get_center() const {return(position + size / 2);}
    Vector2_t<T> get_position() const {return(position);}
    Vector2_t<T> get_size() const {return(size);}
    Vector2_t<T> get_end() const {return(position + size);}

    bool is_regular() const{
        return(this->size.x >= 0 && this->size.y >= 0);
    }
    Rect2_t<T> abs() const {
        if(is_regular()) return(*this);
        T x0 = position.x;
        T x1 = position.x + size.x;
        T y0 = position.y;
        T y1 = position.y + size.y;
        return Rect2_t<T>(MIN(x0, x1), MIN(y0, y1), MAX(x0, x1) - MIN(x0, x1), MAX(y0, y1) - MIN(y0, y1));
    }
    template<typename U>
    bool has_point(const Vector2_t<U> & _point) const {
        Rect2_t<T> regular = *this.abs();
        T x0 = regular.position.x;
        T x1 = regular.position.x + regular.size.x;
        T y0 = regular.position.y;
        T y1 = regular.position.y + regular.size.y;
        Vector2_t<T> point = _point;
        T x = point.x;
        T y = point.y;
        return(x0 <= x && x <= x1 && y0 <= y && y <= y1);
    }

    template<typename U>
    bool contains(const Rect2_t<U> & other)const {
        Rect2_t<T> regular = *this.abs();
        Rect2_t<T> other_regular = other.abs();
        T x0 = regular.position.x;
        T x1 = regular.position.x + regular.size.x;
        T y0 = regular.position.y;
        T y1 = regular.position.y + regular.size.y;
        T x0_other = other_regular.position.x;
        T x1_other = other_regular.position.x + other_regular.size.x;
        T y0_other = other_regular.position.y;
        T y1_other = other_regular.position.y + other_regular.size.y;
        return(x1 <= x1_other && x0 >= x0_other && y1 <= y1_other && y0 >= y0_other);
    }

    template<typename U>
    Rect2_t<T> & operator=(const Rect2_t<U> & other){
        this->position = other.position;
        this->size = other.size;
        return(*this);
    }

    template<typename U>
    bool intersects(const Rect2_t<U> other) const{
        Rect2_t<T> regular = *this.abs();
        Rect2_t<T> other_regular = other.abs();
        T x0 = regular.position.x;
        T x1 = regular.position.x + regular.size.x;
        T y0 = regular.position.y;
        T y1 = regular.position.y + regular.size.y;
        T x0_other = other_regular.position.x;
        T x1_other = other_regular.position.x + other_regular.size.x;
        T y0_other = other_regular.position.y;
        T y1_other = other_regular.position.y + other_regular.size.y;
        return(x1 <= x0_other || x0 >= x1_other || y1 <= y0_other || y0 >= y1_other);
    }
    template<typename U>
    Rect2_t<T> intersection(const Rect2_t<U> & other) const{
        if(!this->intersects(other)) return Rect2_t<T>();
        Rect2_t<T> regular = *this.abs();
        Rect2_t<T> other_regular = other.abs();
        T x0 = regular.position.x;
        T x1 = regular.position.x + regular.size.x;
        T y0 = regular.position.y;
        T y1 = regular.position.y + regular.size.y;
        T x0_other = other_regular.position.x;
        T x1_other = other_regular.position.x + other_regular.size.x;
        T y0_other = other_regular.position.y;
        T y1_other = other_regular.position.y + other_regular.size.y;
        return Rect2_t<T>(MAX(x0, x0_other), MAX(y0, y0_other), MIN(x1, x1_other) - MAX(x0, x0_other), MIN(y1, y1_other) - MAX(y0, y0_other));
    }


    template<typename U>
    Rect2_t<T> merge(const Rect2_t<U> & other) const{
        Rect2_t<T> regular = *this.abs();
        Rect2_t<T> other_regular = other.abs();
        T x0 = regular.position.x;
        T x1 = regular.position.x + regular.size.x;
        T y0 = regular.position.y;
        T y1 = regular.position.y + regular.size.y;
        T x0_other = other_regular.position.x;
        T x1_other = other_regular.position.x + other_regular.size.x;
        T y0_other = other_regular.position.y;
        T y1_other = other_regular.position.y + other_regular.size.y;
        return Rect2_t<T>(MIN(x0, x0_other), MIN(y0, y0_other), MAX(x1, x1_other) - MIN(x0, x0_other), MAX(y1, y1_other) - MIN(y0, y0_other));
    }

    template<typename U>
    Rect2_t<T> scale(const U & amount)const {
        Rect2_t<T> regular = *this.abs();
        Rect2_t<T> ret = Rect2_t<T>(regular.get_center(), regular.size * amount);
        if(ret.is_regular())return ret;
        else return Rect2_t<T>();
    }

    template<typename U>
    Rect2_t<T> grow(const U & amount)const {
        Rect2_t<T> regular = *this.abs();
        Rect2_t<T> ret = Rect2_t<T>(regular.position - amount * Vector2_t<T>(1,1), regular.size + amount * Vector2_t<T>(2,2));
        if(ret.is_regular())return ret;
        else return Rect2_t<T>();
    }

    template<typename U>
    Rect2_t<T> move(const Vector2_t<U> offset)const{
        return Rect2_t<T>(position + offset, size);
    }

    explicit operator bool() const {
        return(bool(size.x) && bool(size.y));
    }
};

typedef Rect2_t<int> Rect2i;
#endif