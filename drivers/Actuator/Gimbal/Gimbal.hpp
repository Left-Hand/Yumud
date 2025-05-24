#pragma once

#include "core/math/real.hpp"
#include "vector2/vector2.hpp"

namespace ymd{

using Vector2 = Vector2<real_t>;

class Gimbal{
protected:
    union{
        struct{
            real_t yaw;
            real_t pitch;
        };
        Vector2 orientation;
    };

    virtual void move() = 0;
public:
    Gimbal(){;}
    ~Gimbal(){;}

    void setOrientation(const real_t _yaw, const real_t _pitch){
        yaw = _yaw;
        pitch = _pitch;
        move();
    }

    void setOrientation(const Vector2 & _orientation){
        setOrientation(_orientation.x, _orientation.y);
    }
}

}