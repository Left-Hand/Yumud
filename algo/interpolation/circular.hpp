#pragma once

// #include "interpolation.hpp"
#include "core/math/real.hpp"
#include "algebra/vectors/vec2.hpp"

namespace ymd::intp{

struct [[nodiscard]] ArcInterpolation final {
protected:
    iq16 m_Centerx;
    iq16 m_Centery;
    iq16 m_dRadius;
    iq16 _a;
    iq16 _b;

    bool IsPerpendicular(const math::Vec2<iq16> & pt1,const math::Vec2<iq16> & pt2,const math::Vec2<iq16> & pt3) const;

    std::tuple<iq16, iq16, iq16> calcCircleFrom3Points (const math::Vec2<iq16> & pt1,const math::Vec2<iq16> & pt2,const math::Vec2<iq16> & pt3) const;
public:
    ArcInterpolation(const math::Vec2<iq16> & handle);

    iq16 forward(const iq16 x) const ;
};

struct [[nodiscard]] CircularFilletInterpoation final {
public:
    CircularFilletInterpoation(const math::Vec2<iq16> & handle, iq16 R):
        _a(handle.x), _b(handle.y), _R(R){
            computeFilletParameters ({0,0}, {_a,_b}, {_a,_b}, {1,1}, _R);
        }


    iq16 forward(iq16 x) const ;

private:
    iq16 arcCenterX = 0;
    iq16 arcCenterY = 0;
    iq16 arcStartAngle = 0;
    iq16 arcEndAngle = 0;
    iq16 arcRadius = 0;
    iq16 arcStartX = 0;
    iq16 arcStartY = 0;
    iq16 arcEndX = 0;
    iq16 arcEndY = 0;

    void computeFilletParameters (
        const math::Vec2<iq16> & pt1,
        const math::Vec2<iq16> & pt2,
        const math::Vec2<iq16> & pt3,
        const math::Vec2<iq16> & pt4,
        iq16 r);

    const iq16 _a;
    const iq16 _b;
    const iq16 _R;
};

};