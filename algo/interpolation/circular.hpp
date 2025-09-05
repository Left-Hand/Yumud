#pragma once

#include "interpolation.hpp"

namespace ymd::intp{

class ArcInterpolation:public Interpolation{
protected:
    real_t m_Centerx;
    real_t m_Centery;
    real_t m_dRadius;
    real_t _a;
    real_t _b;
    // Vec2<q16> 
    bool IsPerpendicular(const Vec2<q16> & pt1,const Vec2<q16> & pt2,const Vec2<q16> & pt3) const;

    std::tuple<real_t, real_t, real_t> calcCircleFrom3Points (const Vec2<q16> & pt1,const Vec2<q16> & pt2,const Vec2<q16> & pt3) const;
public:
    ArcInterpolation(const Vec2<q16> & handle);

    real_t forward(const real_t x) const override;
};

class CircularFilletInterpoation:public Interpolation{
public:
    CircularFilletInterpoation(const Vec2<q16> & handle, real_t R):
        _a(handle.x), _b(handle.y), _R(R){
            computeFilletParameters ({0,0}, {_a,_b}, {_a,_b}, {1,1}, _R);
        }


    real_t forward(real_t x) const override;

private:
    real_t arcCenterX = 0;
    real_t arcCenterY = 0;
    real_t arcStartAngle = 0;
    real_t arcEndAngle = 0;
    real_t arcRadius = 0;
    real_t arcStartX = 0;
    real_t arcStartY = 0;
    real_t arcEndX = 0;
    real_t arcEndY = 0;

    void computeFilletParameters (
        const Vec2<q16> & pt1,
        const Vec2<q16> & pt2,
        const Vec2<q16> & pt3,
        const Vec2<q16> & pt4,
        real_t r);

    const real_t _a;
    const real_t _b;
    const real_t _R;
};

};