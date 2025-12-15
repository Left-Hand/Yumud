#include "../core/platform.hpp"

template<arithmetic T>
struct Line_t{
    Vec2<T> from;
    Vec2<T> to;
};


template<arithmetic T>
struct Circle_t{
    Vec2<T> org;
    real_t to;
};


template<arithmetic T>
struct Arc_t:public Circle_t<T>{
    Range2<T> angle;
};

using Line = Line_t<real_t>;
using Circle = Circle_t<real_t>;
using Arc = Arc_t<real_t>;