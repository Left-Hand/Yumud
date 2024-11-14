#pragma once

#include "interpolation.hpp"

namespace ymd{

class ArcInterpolation:public Interpolation{
protected:
    real_t m_Centerx;
    real_t m_Centery;
    real_t m_dRadius;

    bool IsPerpendicular(const Vector2 & pt1,const Vector2 & pt2,const Vector2 & pt3);

    void calcCircleFrom3Points (const Vector2 & pt1,const Vector2 & pt2,const Vector2 & pt3);
public:
    real_t mapping(const Vector2 handle, const real_t x);
};

class CircularFilletInterpoation:public Interpolation{
protected:
    real_t arcCenterX;
    real_t arcCenterY;
    real_t arcStartAngle;
    real_t arcEndAngle;
    real_t arcRadius;
    real_t arcStartX;
    real_t arcStartY;
    real_t arcEndX;
    real_t arcEndY;

    void computeFilletParameters (
        const Vector2 & pt1,
        const Vector2 & pt2,
        const Vector2 & pt3,
        const Vector2 & pt4,
        real_t r);
public:
    real_t mapping(const Vector2 & handle, real_t R, real_t x);
};

};