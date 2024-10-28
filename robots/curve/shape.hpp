#include "../../sys/core/platform.h"

template<arithmetic T>
struct Line_t{
    Vector2_t<T> from;
    Vector2_t<T> to;
};


template<arithmetic T>
struct Circle_t{
    Vector2_t<T> org;
    real_t to;
};


template<arithmetic T>
struct Arc_t:public Circle_t<T>{
    Range_t<T> angle;
};

using Line = Line_t<real_t>;
using Circle = Circle_t<real_t>;
using Arc = Arc_t<real_t>;