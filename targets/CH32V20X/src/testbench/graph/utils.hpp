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

#define vec3_assign(r, v) \
{ \
    r.x = v.x; \
    r.y = v.y; \
    r.z = v.z; \
}
#define vec3_assign_s(v, a) \
{ \
    v.x = a; \
    v.y = a; \
    v.z = a; \
}
#define vec3_assign_s3(v, a, b, c) \
{ \
    v.x = a; \
    v.y = b; \
    v.z = c; \
}
#define vec3_div_assign_s(v, a) \
{ \
    v.x /= a; \
    v.y /= a; \
    v.z /= a; \
}
#define vec3_mul_assign_s(v, a) \
{ \
    v.x *= a; \
    v.y *= a; \
    v.z *= a; \
}
#define vec3_div_assign(v1, v2) \
{ \
    v1.x /= v2.x; \
    v1.y /= v2.y; \
    v1.z /= v2.z; \
}

#define vec3_add_assign(v1, v2) \
{ \
    v1.x += v2.x; \
    v1.y += v2.y; \
    v1.z += v2.z; \
}
#define vec3_mad_assign(v1, v2, v3) \
{ \
    v1.x += v2.x * v3.x; \
    v1.y += v2.y * v3.y; \
    v1.z += v2.z * v3.z; \
}
#define vec3_mad(r, v1, v2, v3) \
{ \
    r.x = v1.x * v2.x + v3.x; \
    r.y = v1.y * v2.y + v3.y; \
    r.z = v1.z * v2.z + v3.z; \
}
#define vec3_mad_s(r, v1, v2, a) \
{ \
    r.x = v1.x + v2.x * a; \
    r.y = v1.y + v2.y * a; \
    r.z = v1.z + v2.z * a; \
}
#define vec3_add(r, v1, v2) \
{ \
    r.x = v1.x + v2.x; \
    r.y = v1.y + v2.y; \
    r.z = v1.z + v2.z; \
}
#define vec3_mul(r, v1, v2) \
{ \
    r.x = v1.x * v2.x; \
    r.y = v1.y * v2.y; \
    r.z = v1.z * v2.z; \
}
#define vec3_mul_s(r, v, a) \
{ \
    r.x = v.x * a; \
    r.y = v.y * a; \
    r.z = v.z * a; \
}
#define vec3_mul_assign_s(v, a) \
{ \
    v.x *= a; \
    v.y *= a; \
    v.z *= a; \
}

#define vec3_rcp(r, v) \
{ \
    r.x = 1.0 / v.x; \
    r.y = 1.0 / v.y; \
    r.z = 1.0 / v.z; \
}


#define vec3_sub(r, v1, v2) \
{ \
	r.x = v1.x - v2.x; \
	r.y = v1.y - v2.y; \
	r.z = v1.z - v2.z; \
}
#define vec3_min(r, v1, v2) \
{ \
    r.x = std::min(v1.x, v2.x); \
    r.y = std::min(v1.y, v2.y); \
    r.z = std::min(v1.z, v2.z); \
}
#define vec3_max(r, v1, v2) \
{ \
    r.x = std::max(v1.x, v2.x); \
    r.y = std::max(v1.y, v2.y); \
    r.z = std::max(v1.z, v2.z); \
}

#define vec3_compMax(v) (std::max(v.x, std::max(v.y, v.z)))
#define vec3_compMin(v) (std::min(v.x, std::min(v.y, v.z)))

#define dot(v1, v2) (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)
#define dot_m(v1, v2) (-v1.x * v2.x + -v1.y * v2.y + -v1.z * v2.z)


#define normalize(v) \
{ \
	const real_t inv_r = isqrt(real_t(dot(v, v))); \
	if (inv_r > 0) \
	{ \
		v.x = v.x * inv_r; \
		v.y = v.y * inv_r; \
		v.z = v.z * inv_r; \
	} \
}

#define vec3_minus(r, v) \
{ \
    r.x = -v.x; \
    r.y = -v.y; \
    r.z = -v.z; \
}

#define lookat(view, eye, center, up) \
{ \
    const auto zaxis = (eye - center).normalized(); \
 \
	const auto xaxis = up.cross(zaxis).normalized(); \
 \
	const auto yaxis = zaxis.cross(xaxis); \
 \
	view.m[0][0] = xaxis.x; \
	view.m[0][1] = yaxis.x; \
	view.m[0][2] = zaxis.x; \
	view.m[0][3] = 0; \
 \
	view.m[1][0] = xaxis.y; \
	view.m[1][1] = yaxis.y; \
	view.m[1][2] = zaxis.y; \
	view.m[1][3] = 0; \
 \
	view.m[2][0] = xaxis.z; \
	view.m[2][1] = yaxis.z; \
	view.m[2][2] = zaxis.z; \
	view.m[2][3] = 0; \
 \
	view.m[3][0] = -dot(xaxis, eye); \
	view.m[3][1] = -dot(yaxis, eye); \
	view.m[3][2] = -dot(zaxis, eye); \
	view.m[3][3] = 1; \
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

