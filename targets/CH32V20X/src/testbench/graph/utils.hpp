#pragma once

#include <cmath>
#include "core/math/realmath.hpp"
#include "dsp/siggen/noise/LCGNoiseSiggen.hpp"
// #include "dsp/siggen/noise/GaussianNoiseSiggen.hpp"
#include "types.hpp"
#include "core/clock/time.hpp"


scexpr auto vec3_compMax(auto v) {return (std::max(v.x, std::max(v.y, v.z)));}
scexpr auto vec3_compMin(auto v) {return (std::min(v.x, std::min(v.y, v.z)));}




template<typename T>
scexpr mat4_t<T> lookat(const Vector3_t<auto> & eye,const Vector3_t<auto> & center,const Vector3_t<auto> & up){
    const auto zaxis = (eye - center).normalized();
	const auto xaxis = up.cross(zaxis).normalized();
	const auto yaxis = zaxis.cross(xaxis);

    return mat4_t<T>{
        std::array<real_t, 4>{xaxis.x, yaxis.x, zaxis.z, 0},
        std::array<real_t, 4>{xaxis.y, yaxis.y, zaxis.y, 0},
        std::array<real_t, 4>{xaxis.z, yaxis.z, zaxis.z, 0},
        std::array<real_t, 4>{xaxis.dot(eye), yaxis.dot(eye), zaxis.dot(eye), 0}
    };
}


template<typename T>
scexpr mat4_t<T> orthonormalBasis(Vector3_t<T> N){
    const T sign = N.z > 0 ? 1 : -1;
    const T a = -1 / (sign + N.z);
    const T b = N.x * N.y * a;

    return mat4_t<T>{
        std::array<real_t, 4>{1 + sign * N.x * N.x * a, sign * b, -sign * N.x, 0},
        std::array<real_t, 4>{b, sign + N.y * N.y * a, -N.y, 0},
        std::array<real_t, 4>{N.x, N.y, N.z, 0},
        std::array<real_t, 4>{0, 0, 0, 0}
    };
}

static real_t rand01(){
    static dsp::LCGNoiseSiggen ng;
    ng.update();
    return real_t(frac(real_t(ng.get()) >> 16));
}



scexpr auto eye     = Vector3_t<real_t>(0.0_r, 1.0_r, 3.5_r);
scexpr auto center  = Vector3_t<real_t>(0.0_r, 1.0_r, 0.0_r);
scexpr auto up      = Vector3_t<real_t>(0.0_r, 1.0_r, 0.0_r);
scexpr mat4_t view = lookat<real_t>(eye, center, up);;

scexpr auto view_x = Vector3_t<real_t>(view[0][0], view[0][1], view[0][2]);
scexpr auto view_y = Vector3_t<real_t>(view[1][0], view[1][1], view[1][2]);
scexpr auto view_z = Vector3_t<real_t>(view[2][0], view[2][1], view[2][2]);

scexpr auto lightColor = RGB{50, 50, 50};

scexpr auto bbmin = Vector3_t<real_t>(-1, 0, -1);
scexpr auto bbmax = Vector3_t<real_t>(1, 2, 1);


scexpr size_t LCD_W = 160;
scexpr size_t LCD_H = 80;

scexpr real_t INV_LCD_W = real_t(1) / LCD_W;
scexpr real_t INV_LCD_H = real_t(1) / LCD_H;

scexpr size_t max_depth = 2;
scexpr size_t spp  = 1;
scexpr real_t inv_spp  = 1.0_r/spp;


scexpr real_t INV_PI       = 0.318310_r;
scexpr real_t EPSILON      = 0.001_r;
scexpr real_t light_area   = 0.0393_r;
scexpr uint alpha       = 45;

