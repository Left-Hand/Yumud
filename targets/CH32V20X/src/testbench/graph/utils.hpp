#pragma once

#include <cmath>
#include "core/math/realmath.hpp"
#include "core/math/real.hpp"
#include "float.h"

#include "types/vector3/vector3.hpp"

template<size_t Q>
float operator * (const float lhs, const iq_t<Q> rhs){
    return float(real_t(lhs) * rhs);
}


struct mat4_t
{
    float m[4][4];
};


#define vec3_compMax(v) (std::max(v.x, std::max(v.y, v.z)))
#define vec3_compMin(v) (std::min(v.x, std::min(v.y, v.z)))


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


#define orthonormalBasis(N) \
{ \
    const float sign = N.z > 0 ? 1.0f : -1.0f; \
    const float a = -1.0f / (sign + N.z); \
    const float b = N.x * N.y * a; \
 \
    T.m[0][0] = 1.0f + sign * N.x * N.x * a; \
    T.m[0][1] = sign * b; \
    T.m[0][2] = -sign * N.x; \
    T.m[0][3] = 0; \
 \
    T.m[1][0] = b; \
    T.m[1][1] = sign + N.y * N.y * a; \
    T.m[1][2] = -N.y; \
    T.m[1][3] = 0; \
 \
    T.m[2][0] = N.x; \
    T.m[2][1] = N.y; \
    T.m[2][2] = N.z; \
    T.m[2][3] = 0; \
}

