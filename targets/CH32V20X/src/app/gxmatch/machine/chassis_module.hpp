#pragma once

#include "motion_module.hpp"


namespace gxm{
class ChassisModule:public MotionModule{
public:
    void positionTrim(const Vector2 & trim);
    void rotationTrim(const real_t raderr);
    void forwardMove(const Vector2 & pos, const real_t rad);

    void calibrateRotation();
    void calibratePosition();

    void tick();
};

}