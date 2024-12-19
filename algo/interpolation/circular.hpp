#pragma once

#include "interpolation.hpp"

namespace ymd{

class ArcInterpolation:public Interpolation{
protected:
    real_t m_Centerx;
    real_t m_Centery;
    real_t m_dRadius;
    real_t _a;
    real_t _b;
    // Vector2 
    bool IsPerpendicular(const Vector2 & pt1,const Vector2 & pt2,const Vector2 & pt3) const;

    std::tuple<real_t, real_t, real_t> calcCircleFrom3Points (const Vector2 & pt1,const Vector2 & pt2,const Vector2 & pt3) const;
public:
    ArcInterpolation(const Vector2 & handle);

    real_t forward(const real_t x) const override;
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

    const real_t _a;
    const real_t _b;
    const real_t _R;
public:
    CircularFilletInterpoation(const Vector2 & handle, real_t R):
        _a(handle.x), _b(handle.y), _R(R){
            // computeFilletParameters
            computeFilletParameters ({0,0}, {_a,_b}, {_a,_b}, {1,1}, _R);
        }


    real_t forward(real_t x) const override;
};

};