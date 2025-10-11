#pragma once

#include "core/math/real.hpp"
#include "types/regions/ray2.hpp"

namespace ymd::robots{

//序列器约束
struct SequenceLimits{
    q16 max_gyr;
    q16 max_agr;
    q16 max_spd;
    q16 max_acc;
};

//序列器参数
struct SequenceParas{
    size_t freq;
};

//压缩储存的位置和朝向 与Ray互转
struct CurvePoint {
protected:
    static constexpr size_t XY_SHIFT_BITS = 10;
    static constexpr q16 RAD_SCALE = q16(1024 / TAU);
    static constexpr q16 INV_RAD_SCALE = q16(TAU / 1024);
    
    uint32_t x_:11;
    uint32_t y_:11;
    uint32_t rad_:10;
public:
    constexpr CurvePoint(const q16 x, const q16 y, const Angle<q16> orientation):
        x_(x << XY_SHIFT_BITS), y_(y << XY_SHIFT_BITS), rad_(orientation.to_radians() * RAD_SCALE){;}

    constexpr CurvePoint(const Vec2<q16> & pos, const Angle<q16> orientation):
        CurvePoint(pos.x, pos.y, orientation){;}
        
    constexpr CurvePoint(const Ray2<q16> & ray):
        CurvePoint(ray.org.x, ray.org.y, ray.orientation){;}

    constexpr Ray2<q16> to_ray() const{
        return Ray2<q16>(
            Vec2<q16>(
                q16(x_) >> XY_SHIFT_BITS, 
                q16(y_) >> XY_SHIFT_BITS), 
            Angle<q16>::from_radians(rad_ * INV_RAD_SCALE)
        );
    }
};


using Curve = std::vector<CurvePoint>;

}