#pragma once

#include "geometry/GeometryUtils.hpp"

namespace ymd::robots{

//序列器约束
struct SequenceLimits{
    real_t max_gyr;
    real_t max_agr;
    real_t max_spd;
    real_t max_acc;
};

//序列器参数
struct SequenceParas{
    size_t freq;
};

//压缩储存的位置和朝向 与Ray互转
struct CurvePoint {
protected:
    scexpr int XY_SHIFT_BITS = 10;
    scexpr real_t RAD_SCALE = real_t(1024 / TAU);
    scexpr real_t INV_RAD_SCALE = real_t(TAU / 1024);
    
    uint32_t x_:11;
    uint32_t y_:11;
    uint32_t rad_:10;
public:
    constexpr CurvePoint(const real_t & x, const real_t & y, const real_t & rad):
        x_(x << XY_SHIFT_BITS), y_(y << XY_SHIFT_BITS), rad_(rad * RAD_SCALE){;}

    constexpr CurvePoint(const Vector2 & pos, const real_t & rad):
        CurvePoint(pos.x, pos.y, rad){;}
        
    constexpr CurvePoint(const Ray2_t<real_t> & ray):
        CurvePoint(ray.org.x, ray.org.y, ray.rad){;}

    constexpr Ray2_t<real_t> to_ray() const{
        return Ray2_t<real_t>(
            Vector2_t<real_t>(
                real_t(x_) >> XY_SHIFT_BITS, 
                real_t(y_) >> XY_SHIFT_BITS), 
            rad_ * INV_RAD_SCALE
        );
    }
};


using Curve = std::vector<CurvePoint>;

}