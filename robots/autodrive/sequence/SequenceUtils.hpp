#pragma once

#include "core/math/real.hpp"
#include "algebra/regions/ray2.hpp"

namespace ymd::robots{

//序列器约束
struct SequenceLimits{
    iq16 max_gyr;
    iq16 max_agr;
    iq16 max_spd;
    iq16 max_acc;
};

//序列器参数
struct SequenceParas{
    size_t freq;
};

//压缩储存的位置和朝向 与Ray互转
struct CurvePoint {
protected:
    static constexpr size_t XY_SHIFT_BITS = 10;
    static constexpr iq16 RAD_SCALE = iq16(1024 / TAU);
    static constexpr iq16 INV_RAD_SCALE = iq16(TAU / 1024);
    
    uint32_t x_:11;
    uint32_t y_:11;
    uint32_t radians_:10;
public:
    constexpr CurvePoint(const iq16 x, const iq16 y, const Angular<iq16> orientation):
        x_(x << XY_SHIFT_BITS), y_(y << XY_SHIFT_BITS), radians_(orientation.to_radians() * RAD_SCALE){;}

    constexpr CurvePoint(const math::Vec2<iq16> & pos, const Angular<iq16> orientation):
        CurvePoint(pos.x, pos.y, orientation){;}
        
    constexpr CurvePoint(const math::Ray2<iq16> & ray):
        CurvePoint(ray.center.x, ray.center.y, ray.orientation){;}

    constexpr math::Ray2<iq16> to_ray() const{
        return math::Ray2<iq16>(
            math::Vec2<iq16>(
                iq16(x_) >> XY_SHIFT_BITS, 
                iq16(y_) >> XY_SHIFT_BITS), 
            Angular<iq16>::from_radians(radians_ * INV_RAD_SCALE)
        );
    }
};


using Curve = std::vector<CurvePoint>;

}