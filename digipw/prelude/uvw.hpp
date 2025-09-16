#pragma once

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"
#include "core/math/realmath.hpp"

namespace ymd::digipw{

template<typename T>
struct UvwCoord{
    T u;
    T v;
    T w;

    static constexpr UvwCoord<T> ZERO = {T(0), T(0), T(0)};

    constexpr T operator [](const size_t idx) const {
        return *(&u + idx);
    }

    constexpr T & operator [](const size_t idx){
        return *(&u + idx);
    }

    constexpr T numeric_sum() const {
        return u + v + w;
    }

    friend OutputStream & operator << (OutputStream & os, const UvwCoord & self){
        return os << os.brackets<'('>() << 
            self.u << os.splitter() << 
            self.v <<  os.splitter() << 
            self.w << os.brackets<')'>();
    }
};

}