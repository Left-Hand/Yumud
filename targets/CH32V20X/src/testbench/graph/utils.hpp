#pragma once

#include <cmath>
#include "core/math/realmath.hpp"
#include "dsp/siggen/noise/LCGNoiseSiggen.hpp"
// #include "dsp/siggen/noise/GaussianNoiseSiggen.hpp"
#include "types.hpp"
#include "core/clock/time.hpp"

template<size_t Q>
float operator * (const float lhs, const iq_t<Q> rhs){
    return float(real_t(lhs) * rhs);
}



scexpr auto vec3_compMax(auto v) {return (std::max(v.x, std::max(v.y, v.z)));}
scexpr auto vec3_compMin(auto v) {return (std::min(v.x, std::min(v.y, v.z)));}


scexpr mat4_t lookat(const Vector3_t<auto> eye,const Vector3_t<auto>  center,const Vector3_t<auto>  up){
    const auto zaxis = (eye - center).normalized();
	const auto xaxis = up.cross(zaxis).normalized();
	const auto yaxis = zaxis.cross(xaxis);

    mat4_t view;

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


scexpr mat4_t orthonormalBasis(Vector3_t<float> N){
    const float sign = N.z > 0 ? 1.0f : -1.0f;
    const float a = -1.0f / (sign + N.z);
    const float b = N.x * N.y * a;

    mat4_t T;

    T.m[0][0] = 1.0f + sign * N.x * N.x * a;
    T.m[0][1] = sign * b;
    T.m[0][2] = -sign * N.x;
    T.m[0][3] = 0;

    T.m[1][0] = b;
    T.m[1][1] = sign + N.y * N.y * a;
    T.m[1][2] = -N.y;
    T.m[1][3] = 0;

    T.m[2][0] = N.x;
    T.m[2][1] = N.y;
    T.m[2][2] = N.z;
    T.m[2][3] = 0;

    return T;
}

static real_t rand01(){
    static dsp::LCGNoiseSiggen ng;
    ng.update();
    return real_t(frac(float(ng.get()) / 65536));
}
// static real_t rand01(){
//     return sinpu(80 * time()) * 0.5_r + 0.5_r;
// }

// #define rand01() (float(rand()) / RAND_MAX)
// #define rand01() (0.01_r)


static float bsdf_pdf;


scexpr auto eye     = Vector3_t(0.0f, 1.0f, 3.5f);
scexpr auto center  = Vector3_t(0.0f, 1.0f, 0.0f);
scexpr auto up      = Vector3_t(0.0f, 1.0f, 0.0f);
scexpr mat4_t view = lookat(eye, center, up);;

scexpr auto view_x = Vector3_t(view.m[0][0], view.m[0][1], view.m[0][2]);
scexpr auto view_y = Vector3_t(view.m[1][0], view.m[1][1], view.m[1][2]);
scexpr auto view_z = Vector3_t(view.m[2][0], view.m[2][1], view.m[2][2]);

// scexpr auto lightColor = Vector3_t<float>::from_ones(16);
scexpr auto lightColor = Vector3_t<float>::from_ones(50);

scexpr auto bbmin = Vector3_t<float>(-1, 0, -1);
scexpr auto bbmax = Vector3_t<float>(1, 2, 1);


scexpr size_t LCD_W = 240;
scexpr size_t LCD_H = 135;

scexpr size_t max_depth = 2;
scexpr size_t spp  = 1;
scexpr float inv_spp  = float(1)/spp;


scexpr float INV_PI       = 0.318310f;
scexpr float EPSILON      = 0.001f;
scexpr float light_area   = 0.0393f;
scexpr float alpha       = 45;

static struct intersection_t intersection;
static const struct triangle_t* s;
// static const mat4_t T;
// static struct ray_t ray;

static Vector3_t<float> Reflectance(int8_t i)
{
    if (i == 8 || i == 9){
        return Vector3_t(0.05f, 0.65f, 0.05f);
    }
    else if (i == 10 || i == 11){
        return Vector3_t(0.65f, 0.05f, 0.05f);
    }
    else{
        return Vector3_t<float>::from_ones(0.65f);
    }
}

