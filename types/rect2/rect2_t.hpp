#ifndef __RECT2_HPP__

#define __RECT2_HPP__

#include "../vector2/Vector2.hpp"
#include "../../src/defines/comm_inc.h"

template<typename T>
class Rect2_t{
private:
    Vector2_t<T> position = Vector2_t<T>();
    Vector2_t<T> size = Vector2_t<T>;

    Rect2_t(){;}

    template<typename U>
    Rect2_t(const Rect2_t<U> other):position(other.position), size(other.size){;}

    template<typename U>
    Rect2_t(const Vector2_t<U> & _position,const Vector2_t<U> & _size):position(_position), size(_size){;}
    
    template<typename U>
    Rect2_t(U x, U y, U width, U height):position(Vector2_t<U>(x,y)),size(Vector2_t<U>(width, height)){;} 

    T get_area(){return size.x * size.y;}
    Vector2_t<T> get_center() const {return(position + size / 2);}
    Rect2_t<T> grow(const T & amount){return(Rect2_t<T>(get_center() - amount * size / 2, size * amount));}
    Rect2_t<T> abs() const;

    template<typename U>
    bool has_point(const Vector2_t<U> point)const;
    Rect_2<T> merge(const Rect_2<U> &_p_rect) const;
}


#include "rect2_t.tpp"

#endif