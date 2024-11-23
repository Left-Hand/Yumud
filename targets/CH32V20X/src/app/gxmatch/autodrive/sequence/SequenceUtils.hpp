#pragma once

#include "geometry/GeometryUtils.hpp"

namespace gxm{

using Ray = ymd::Ray2D_t<real_t>;
using Line = ymd::Line2D_t<real_t>;
using Vector2 = ymd::Vector2_t<real_t>;
using Vector3 = ymd::Vector3_t<real_t>;
using Quat = ymd::Quat_t<real_t>;
using Basis = ymd::Basis_t<real_t>;

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
    scexpr int xy_shift_bits = 10;
    scexpr real_t rad_scale = real_t(1024 / TAU);
    scexpr real_t inv_rad_scale = real_t(TAU / 1024);
    
    uint32_t x_:11;
    uint32_t y_:11;
    uint32_t rad_:10;
public:
    constexpr CurvePoint(const real_t & x, const real_t & y, const real_t & rad):
        x_(x << xy_shift_bits), y_(y << xy_shift_bits), rad_(rad * rad_scale){;}

    constexpr CurvePoint(const Vector2 & pos, const real_t & rad):
        CurvePoint(pos.x, pos.y, rad){;}
        
    constexpr CurvePoint(const Ray & ray):
        CurvePoint(ray.org.x, ray.org.y, ray.rad){;}


    constexpr operator Ray() const{
        return Ray(ymd::Vector2(real_t(x_) >> xy_shift_bits, real_t(y_) >> xy_shift_bits), rad_ * inv_rad_scale);
    }
};

// struct CurvePoint {
// protected:
//     scexpr int xy_scale = 1000;
//     scexpr real_t rad_scale = real_t(256 / TAU);
//     scexpr real_t inv_rad_scale = real_t(TAU / 256);
    
//     uint32_t x_:12;
//     uint32_t y_:12;
//     uint32_t rad_:8;
// public:
//     constexpr CurvePoint(const real_t & x, const real_t & y, const real_t & rad):
//         x_(x * xy_scale), y_(y * xy_scale), rad_(rad * rad_scale){;}
//     constexpr CurvePoint(const Vector2 & pos, const real_t & rad):
//         CurvePoint(pos.x, pos.y, rad){;}
        
//     constexpr CurvePoint(const Ray & ray):
//         CurvePoint(ray.org.x, ray.org.y, ray.rad){;}


//     constexpr operator Ray() const{
//         return Ray(ymd::Vector2(real_t(x_) / xy_scale, real_t(y_) / xy_scale), rad_ * inv_rad_scale);
//     }
// };




__fast_inline ymd::OutputStream & operator<<(ymd::OutputStream & os, const gxm::CurvePoint & gesture){
    return os << gxm::Ray(gesture);
}

using Curve = std::vector<CurvePoint>;




}