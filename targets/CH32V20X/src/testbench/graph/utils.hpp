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

    mat4_t<T> view;

	view.m[0][0] = xaxis.x;
	view.m[0][1] = yaxis.x;
	view.m[0][2] = zaxis.x;
	view.m[0][3] = 0;

	view.m[1][0] = xaxis.y;
	view.m[1][1] = yaxis.y;
	view.m[1][2] = zaxis.y;
	view.m[1][3] = 0;

	view.m[2][0] = xaxis.z;
	view.m[2][1] = yaxis.z;
	view.m[2][2] = zaxis.z;
	view.m[2][3] = 0;

	view.m[3][0] = -xaxis.dot(eye);
	view.m[3][1] = -yaxis.dot(eye);
	view.m[3][2] = -zaxis.dot(eye);
	view.m[3][3] = 1;

    return view;
}


template<typename T>
scexpr mat4_t<T> orthonormalBasis(Vector3_t<T> N){
    const T sign = N.z > 0 ? 1 : -1;
    const T a = -1 / (sign + N.z);
    const T b = N.x * N.y * a;

    mat4_t<T> transform;

    transform.m[0][0] = 1 + sign * N.x * N.x * a;
    transform.m[0][1] = sign * b;
    transform.m[0][2] = -sign * N.x;
    transform.m[0][3] = 0;

    transform.m[1][0] = b;
    transform.m[1][1] = sign + N.y * N.y * a;
    transform.m[1][2] = -N.y;
    transform.m[1][3] = 0;

    transform.m[2][0] = N.x;
    transform.m[2][1] = N.y;
    transform.m[2][2] = N.z;
    transform.m[2][3] = 0;

    return transform;
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

scexpr auto view_x = Vector3_t<real_t>(view.m[0][0], view.m[0][1], view.m[0][2]);
scexpr auto view_y = Vector3_t<real_t>(view.m[1][0], view.m[1][1], view.m[1][2]);
scexpr auto view_z = Vector3_t<real_t>(view.m[2][0], view.m[2][1], view.m[2][2]);

scexpr auto lightColor = Vector3_t<real_t>::from_ones(50);

scexpr auto bbmin = Vector3_t<real_t>(-1, 0, -1);
scexpr auto bbmax = Vector3_t<real_t>(1, 2, 1);


scexpr size_t LCD_W = 160;
scexpr size_t LCD_H = 80;

scexpr size_t max_depth = 2;
scexpr size_t spp  = 1;
scexpr real_t inv_spp  = 1.0_r/spp;


scexpr real_t INV_PI       = 0.318310_r;
scexpr real_t EPSILON      = 0.001_r;
scexpr real_t light_area   = 0.0393_r;
scexpr uint alpha       = 45;

scexpr Vector3_t<real_t> Reflectance(int8_t i)
{
    if (i == 8 || i == 9){
        return Vector3_t(0.05_r, 0.65_r, 0.05_r);
    }
    else if (i == 10 || i == 11){
        return Vector3_t(0.65_r, 0.05_r, 0.05_r);
    }
    else{
        return Vector3_t<real_t>::from_ones(0.65_r);
    }
}

