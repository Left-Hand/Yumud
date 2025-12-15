#pragma once

#include "vec2.hpp"

namespace ymd{


template<typename T>
struct [[nodiscard]] Polar{
    using Self = Polar;

    T amplitude;
    Angular<T> phase;
    static constexpr Polar<T> zero(){
        return Polar<T>{
            T(0), 
            Angular<T>::ZERO
        };
    }

    static constexpr Polar<T> from_vec2(const Vec2<T>& vec) {
        return Polar<T>{vec.length(), vec.angle()};
    }

    [[nodiscard]] constexpr Vec2<T> to_vec2() const {
        return Vec2<T>::from_angle_and_length(phase, amplitude);
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os    
            << os.field("amplitude")(self.amplitude) << os.splitter()
            << os.field("phase")(self.phase.to_radians())
        ;
    }
};


}