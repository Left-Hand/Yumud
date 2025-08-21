#pragma once

#include "geometry/GeometryUtils.hpp"

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
    scexpr int XY_SHIFT_BITS = 10;
    scexpr q16 RAD_SCALE = q16(1024 / TAU);
    scexpr q16 INV_RAD_SCALE = q16(TAU / 1024);
    
    uint32_t x_:11;
    uint32_t y_:11;
    uint32_t rad_:10;
public:
    constexpr CurvePoint(const q16 & x, const q16 & y, const q16 & rad):
        x_(x << XY_SHIFT_BITS), y_(y << XY_SHIFT_BITS), rad_(rad * RAD_SCALE){;}

    constexpr CurvePoint(const Vec2<q16> & pos, const q16 & rad):
        CurvePoint(pos.x, pos.y, rad){;}
        
    constexpr CurvePoint(const Ray2<q16> & ray):
        CurvePoint(ray.org.x, ray.org.y, ray.rad){;}

    constexpr Ray2<q16> to_ray() const{
        return Ray2<q16>(
            Vec2<q16>(
                q16(x_) >> XY_SHIFT_BITS, 
                q16(y_) >> XY_SHIFT_BITS), 
            rad_ * INV_RAD_SCALE
        );
    }
};


using Curve = std::vector<CurvePoint>;

}