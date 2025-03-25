#pragma once

#include <cmath>
#include "core/math/realmath.hpp"

#include "types.hpp"

template<size_t Q>
float operator * (const float lhs, const iq_t<Q> rhs){
    return float(real_t(lhs) * rhs);
}



static constexpr auto vec3_compMax(auto v) {return (std::max(v.x, std::max(v.y, v.z)));}
static constexpr auto vec3_compMin(auto v) {return (std::min(v.x, std::min(v.y, v.z)));}


static constexpr mat4_t lookat(const Vector3_t<auto> eye,const Vector3_t<auto>  center,const Vector3_t<auto>  up){
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


static constexpr mat4_t orthonormalBasis(Vector3_t<float> N){
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

// static real_t rand01(){
//     static dsp::LCGNoiseSiggen noise;
//     iq_t<16> temp;
//     temp.value.from_i32(noise.update());
//     return frac(temp);
// }

#define rand01() (float(rand()) / RAND_MAX)
#define balanceHeuristic(a, b) ((a) / ((a) + (b)))

// #define abs(x) ((x) > 0 ? x : -x)
// static Vector3_t<float> bsdf_absIdotN;
// static float bsdf_pdf;
static const struct triangle_t* s;
static float bsdf_pdf;


static constexpr auto eye     = Vector3_t(0.0f, 1.0f, 3.5f);
static constexpr auto center  = Vector3_t(0.0f, 1.0f, 0.0f);
static constexpr auto up      = Vector3_t(0.0f, 1.0f, 0.0f);
static constexpr mat4_t view = lookat(eye, center, up);;

static constexpr auto view_x = Vector3_t(view.m[0][0], view.m[0][1], view.m[0][2]);
static constexpr auto view_y = Vector3_t(view.m[1][0], view.m[1][1], view.m[1][2]);
static constexpr auto view_z = Vector3_t(view.m[2][0], view.m[2][1], view.m[2][2]);

static constexpr auto lightColor = Vector3_t<float>::from_ones(200);

static constexpr auto bbmin = Vector3_t<float>(-1, 0, -1);
static constexpr auto bbmax = Vector3_t<float>(1, 2, 1);


#define LCD_W 160
#define LCD_H 80

#define alpha 45

#define spp 1
#define max_depth 2


#define INV_PI 0.318310f
#define EPSILON 0.001f
#define light_area 0.0893f

static struct intersection_t intersection;

static mat4_t T;
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

