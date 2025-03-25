/*
* STC8H8K64U-45I-PDIP40 @40MHz
* sdcc 3drt.c --model-large
* https://liuliqiang.com/51/3drt.c
* by liuliqiang 2025-03-23
*/

#include "src/testbench/tb.h"

#include "core/math/realmath.hpp"
#include "core/math/real.hpp"
#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"


#include "drivers/Display/Polychrome/ST7789/st7789.hpp"


using namespace ymd;


static constexpr size_t spp = 5;
static constexpr size_t max_depth = 20;



struct vec3_t
{
    real_t x, y, z;
};

struct mat4_t
{
    real_t m[4][4];
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
#define vec3_mul_assign(v1, v2) \
{ \
    v1.x *= v2.x; \
    v1.y *= v2.y; \
    v1.z *= v2.z; \
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
    r.x = 1.0_r / v.x; \
    r.y = 1.0_r / v.y; \
    r.z = 1.0_r / v.z; \
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

#define cross(r, v1, v2) \
{ \
	r.x = v1.y * v2.z - v1.z * v2.y; \
	r.y = v1.z * v2.x - v1.x * v2.z; \
	r.z = v1.x * v2.y - v1.y * v2.x; \
}

#define normalize(v) \
{ \
	const real_t inv_r = isqrt(dot(v, v)); \
	if (inv_r > 0.0_r) \
	{ \
		v.x *= inv_r; \
		v.y *= inv_r; \
		v.z *= inv_r; \
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
	struct vec3_t zaxis; \
	vec3_sub(zaxis, eye, center); \
	normalize(zaxis); \
 \
	struct vec3_t xaxis; \
	cross(xaxis, up, zaxis); \
	normalize(xaxis); \
 \
	struct vec3_t yaxis; \
	cross(yaxis, zaxis, xaxis); \
 \
	view.m[0][0] = xaxis.x; \
	view.m[0][1] = yaxis.x; \
	view.m[0][2] = zaxis.x; \
	view.m[0][3] = 0.0_r; \
 \
	view.m[1][0] = xaxis.y; \
	view.m[1][1] = yaxis.y; \
	view.m[1][2] = zaxis.y; \
	view.m[1][3] = 0.0_r; \
 \
	view.m[2][0] = xaxis.z; \
	view.m[2][1] = yaxis.z; \
	view.m[2][2] = zaxis.z; \
	view.m[2][3] = 0.0_r; \
 \
	view.m[3][0] = -dot(xaxis, eye); \
	view.m[3][1] = -dot(yaxis, eye); \
	view.m[3][2] = -dot(zaxis, eye); \
	view.m[3][3] = 1.0_r; \
}

static struct mat4_t T;
#define orthonormalBasis(N) \
{ \
    const real_t sign = N.z > 0.0_r ? 1.0_r : -1.0_r; \
    const real_t a = -1.0_r / (sign + N.z); \
    const real_t b = N.x * N.y * a; \
 \
    T.m[0][0] = 1.0_r + sign * N.x * N.x * a; \
    T.m[0][1] = sign * b; \
    T.m[0][2] = -sign * N.x; \
    T.m[0][3] = 0.0_r; \
 \
    T.m[1][0] = b; \
    T.m[1][1] = sign + N.y * N.y * a; \
    T.m[1][2] = -N.y; \
    T.m[1][3] = 0.0_r; \
 \
    T.m[2][0] = N.x; \
    T.m[2][1] = N.y; \
    T.m[2][2] = N.z; \
    T.m[2][3] = 0.0_r; \
}

#define radians(x) ((x) * 0.017453_r)

static const real_t triangles[] =
{
    -0.240000_r, 1.980000_r, -0.220000_r, 0.230000_r, 1.980000_r, 0.160000_r,
    -0.240000_r, 1.980000_r, -0.220000_r, 0.230000_r, 1.980000_r, 0.160000_r, -0.240000_r, 1.980000_r, 0.160000_r,
    0.470000_r, 0.000000_r, 0.380000_r, 0.000000_r, 0.000000_r, 0.380000_r, 0.000000_r, -1.000000_r, 0.000000_r,
    -0.240000_r, 1.980000_r, -0.220000_r, 0.230000_r, 1.980000_r, 0.160000_r,
    -0.240000_r, 1.980000_r, -0.220000_r, 0.230000_r, 1.980000_r, -0.220000_r, 0.230000_r, 1.980000_r, 0.160000_r,
    0.470000_r, 0.000000_r, 0.000000_r, 0.470000_r, 0.000000_r, 0.380000_r, 0.000000_r, -1.000000_r, 0.000000_r,
    -1.010000_r, 0.000000_r, -1.040000_r, 1.000000_r, 0.000000_r, 0.990000_r,
    1.000000_r, 0.000000_r, 0.990000_r, -0.990000_r, -0.000000_r, -1.040000_r, -1.010000_r, 0.000000_r, 0.990000_r,
    -1.990000_r, -0.000000_r, -2.030000_r, -2.010000_r, 0.000000_r, 0.000000_r, 0.000000_r, 1.000000_r, -0.000000_r,
    -1.020000_r, 1.990000_r, -1.040000_r, 1.000000_r, 1.990000_r, 0.990000_r,
    -1.020000_r, 1.990000_r, -1.040000_r, 1.000000_r, 1.990000_r, 0.990000_r, -1.020000_r, 1.990000_r, 0.990000_r,
    2.020000_r, 0.000000_r, 2.030000_r, 0.000000_r, 0.000000_r, 2.030000_r, 0.000000_r, -1.000000_r, 0.000000_r,
    -1.020000_r, -0.000000_r, -1.040000_r, 1.000000_r, 1.990000_r, -1.040000_r,
    -0.990000_r, -0.000000_r, -1.040000_r, 1.000000_r, 1.990000_r, -1.040000_r, -1.020000_r, 1.990000_r, -1.040000_r,
    1.990000_r, 1.990000_r, 0.000000_r, -0.030000_r, 1.990000_r, 0.000000_r, 0.000000_r, -0.000000_r, 1.000000_r,
    -0.990000_r, 0.000000_r, -1.040000_r, 1.000000_r, 0.000000_r, 0.990000_r,
    1.000000_r, 0.000000_r, 0.990000_r, 1.000000_r, -0.000000_r, -1.040000_r, -0.990000_r, -0.000000_r, -1.040000_r,
    0.000000_r, -0.000000_r, -2.030000_r, -1.990000_r, -0.000000_r, -2.030000_r, 0.000000_r, 1.000000_r, -0.000000_r,
    -1.020000_r, 1.990000_r, -1.040000_r, 1.000000_r, 1.990000_r, 0.990000_r,
    -1.020000_r, 1.990000_r, -1.040000_r, 1.000000_r, 1.990000_r, -1.040000_r, 1.000000_r, 1.990000_r, 0.990000_r,
    2.020000_r, 0.000000_r, 0.000000_r, 2.020000_r, 0.000000_r, 2.030000_r, 0.000000_r, -1.000000_r, 0.000000_r,
    -0.990000_r, -0.000000_r, -1.040000_r, 1.000000_r, 1.990000_r, -1.040000_r,
    -0.990000_r, -0.000000_r, -1.040000_r, 1.000000_r, -0.000000_r, -1.040000_r, 1.000000_r, 1.990000_r, -1.040000_r,
    1.990000_r, 0.000000_r, 0.000000_r, 1.990000_r, 1.990000_r, 0.000000_r, 0.000000_r, 0.000000_r, 1.000000_r,
    -1.020000_r, 0.000000_r, -1.040000_r, -1.010000_r, 1.990000_r, 0.990000_r,
    -1.010000_r, 0.000000_r, 0.990000_r, -1.020000_r, 1.990000_r, -1.040000_r, -1.020000_r, 1.990000_r, 0.990000_r,
    -0.010000_r, 1.990000_r, -2.030000_r, -0.010000_r, 1.990000_r, 0.000000_r, 0.999987_r, 0.005025_r, 0.000000_r,
    -1.020000_r, 0.000000_r, -1.040000_r, -0.990000_r, 1.990000_r, 0.990000_r,
    -1.010000_r, 0.000000_r, 0.990000_r, -0.990000_r, -0.000000_r, -1.040000_r, -1.020000_r, 1.990000_r, -1.040000_r,
    0.020000_r, -0.000000_r, -2.030000_r, -0.010000_r, 1.990000_r, -2.030000_r, 0.999838_r, 0.015073_r, 0.009851_r,
    1.000000_r, 0.000000_r, -1.040000_r, 1.000000_r, 1.990000_r, 0.990000_r,
    1.000000_r, 0.000000_r, 0.990000_r, 1.000000_r, 1.990000_r, -1.040000_r, 1.000000_r, -0.000000_r, -1.040000_r,
    0.000000_r, 1.990000_r, -2.030000_r, 0.000000_r, -0.000000_r, -2.030000_r, -1.000000_r, 0.000000_r, -0.000000_r,
    1.000000_r, 0.000000_r, -1.040000_r, 1.000000_r, 1.990000_r, 0.990000_r,
    1.000000_r, 0.000000_r, 0.990000_r, 1.000000_r, 1.990000_r, 0.990000_r, 1.000000_r, 1.990000_r, -1.040000_r,
    0.000000_r, 1.990000_r, 0.000000_r, 0.000000_r, 1.990000_r, -2.030000_r, -1.000000_r, 0.000000_r, 0.000000_r,
    -0.710000_r, 1.200000_r, -0.490000_r, 0.040000_r, 1.200000_r, 0.090000_r,
    0.040000_r, 1.200000_r, -0.090000_r, -0.710000_r, 1.200000_r, -0.490000_r, -0.530000_r, 1.200000_r, 0.090000_r,
    -0.750000_r, 0.000000_r, -0.400000_r, -0.570000_r, 0.000000_r, 0.180000_r, 0.000000_r, 1.000000_r, 0.000000_r,
    -0.710000_r, -0.000000_r, -0.490000_r, -0.530000_r, 1.200000_r, 0.090000_r,
    -0.530000_r, 1.200000_r, 0.090000_r, -0.710000_r, -0.000000_r, -0.490000_r, -0.530000_r, 0.000000_r, 0.090000_r,
    -0.180000_r, -1.200000_r, -0.580000_r, 0.000000_r, -1.200000_r, 0.000000_r, -0.955064_r, 0.000000_r, 0.296399_r,
    -0.710000_r, -0.000000_r, -0.670000_r, -0.140000_r, 1.200000_r, -0.490000_r,
    -0.710000_r, 1.200000_r, -0.490000_r, -0.140000_r, -0.000000_r, -0.670000_r, -0.710000_r, -0.000000_r, -0.490000_r,
    0.570000_r, -1.200000_r, -0.180000_r, 0.000000_r, -1.200000_r, 0.000000_r, -0.301131_r, -0.000000_r, -0.953583_r,
    -0.140000_r, -0.000000_r, -0.670000_r, 0.040000_r, 1.200000_r, -0.090000_r,
    -0.140000_r, 1.200000_r, -0.670000_r, 0.040000_r, -0.000000_r, -0.090000_r, -0.140000_r, -0.000000_r, -0.670000_r,
    0.180000_r, -1.200000_r, 0.580000_r, 0.000000_r, -1.200000_r, 0.000000_r, 0.955064_r, 0.000000_r, -0.296399_r,
    -0.530000_r, 0.000000_r, -0.090000_r, 0.040000_r, 1.200000_r, 0.090000_r,
    0.040000_r, 1.200000_r, -0.090000_r, -0.530000_r, 0.000000_r, 0.090000_r, 0.040000_r, -0.000000_r, -0.090000_r,
    -0.570000_r, -1.200000_r, 0.180000_r, 0.000000_r, -1.200000_r, 0.000000_r, 0.301131_r, 0.000000_r, 0.953583_r,
    -0.710000_r, 1.200000_r, -0.670000_r, 0.040000_r, 1.200000_r, -0.090000_r,
    0.040000_r, 1.200000_r, -0.090000_r, -0.140000_r, 1.200000_r, -0.670000_r, -0.710000_r, 1.200000_r, -0.490000_r,
    -0.180000_r, 0.000000_r, -0.580000_r, -0.750000_r, 0.000000_r, -0.400000_r, 0.000000_r, 1.000000_r, 0.000000_r,
    -0.710000_r, -0.000000_r, -0.490000_r, -0.530000_r, 1.200000_r, 0.090000_r,
    -0.530000_r, 1.200000_r, 0.090000_r, -0.710000_r, 1.200000_r, -0.490000_r, -0.710000_r, -0.000000_r, -0.490000_r,
    -0.180000_r, 0.000000_r, -0.580000_r, -0.180000_r, -1.200000_r, -0.580000_r, -0.955064_r, 0.000000_r, 0.296399_r,
    -0.710000_r, -0.000000_r, -0.670000_r, -0.140000_r, 1.200000_r, -0.490000_r,
    -0.710000_r, 1.200000_r, -0.490000_r, -0.140000_r, 1.200000_r, -0.670000_r, -0.140000_r, -0.000000_r, -0.670000_r,
    0.570000_r, 0.000000_r, -0.180000_r, 0.570000_r, -1.200000_r, -0.180000_r, -0.301131_r, 0.000000_r, -0.953583_r,
    -0.140000_r, -0.000000_r, -0.670000_r, 0.040000_r, 1.200000_r, -0.090000_r,
    -0.140000_r, 1.200000_r, -0.670000_r, 0.040000_r, 1.200000_r, -0.090000_r, 0.040000_r, -0.000000_r, -0.090000_r,
    0.180000_r, 0.000000_r, 0.580000_r, 0.180000_r, -1.200000_r, 0.580000_r, 0.955064_r, 0.000000_r, -0.296399_r,
    -0.530000_r, 0.000000_r, -0.090000_r, 0.040000_r, 1.200000_r, 0.090000_r,
    0.040000_r, 1.200000_r, -0.090000_r, -0.530000_r, 1.200000_r, 0.090000_r, -0.530000_r, 0.000000_r, 0.090000_r,
    -0.570000_r, 0.000000_r, 0.180000_r, -0.570000_r, -1.200000_r, 0.180000_r, 0.301131_r, 0.000000_r, 0.953583_r,
    -0.050000_r, 0.600000_r, -0.000000_r, 0.530000_r, 0.600000_r, 0.750000_r,
    0.530000_r, 0.600000_r, 0.750000_r, 0.130000_r, 0.600000_r, -0.000000_r, -0.050000_r, 0.600000_r, 0.570000_r,
    -0.400000_r, 0.000000_r, -0.750000_r, -0.580000_r, 0.000000_r, -0.180000_r, 0.000000_r, 1.000000_r, 0.000000_r,
    -0.050000_r, 0.000000_r, 0.000000_r, 0.130000_r, 0.600000_r, 0.570000_r,
    -0.050000_r, 0.600000_r, 0.570000_r, 0.130000_r, 0.000000_r, 0.000000_r, -0.050000_r, 0.000000_r, 0.570000_r,
    0.180000_r, -0.600000_r, -0.570000_r, 0.000000_r, -0.600000_r, 0.000000_r, -0.953583_r, -0.000000_r, -0.301131_r,
    -0.050000_r, 0.000000_r, 0.570000_r, 0.530000_r, 0.600000_r, 0.750000_r,
    0.530000_r, 0.600000_r, 0.750000_r, -0.050000_r, 0.000000_r, 0.570000_r, 0.530000_r, 0.000000_r, 0.750000_r,
    -0.580000_r, -0.600000_r, -0.180000_r, 0.000000_r, -0.600000_r, 0.000000_r, -0.296399_r, 0.000000_r, 0.955064_r,
    0.530000_r, 0.000000_r, 0.170000_r, 0.700000_r, 0.600000_r, 0.750000_r,
    0.700000_r, 0.600000_r, 0.170000_r, 0.530000_r, 0.000000_r, 0.750000_r, 0.700000_r, 0.000000_r, 0.170000_r,
    -0.170000_r, -0.600000_r, 0.580000_r, 0.000000_r, -0.600000_r, 0.000000_r, 0.959629_r, 0.000000_r, 0.281270_r,
    0.130000_r, 0.000000_r, -0.000000_r, 0.700000_r, 0.600000_r, 0.170000_r,
    0.130000_r, 0.600000_r, -0.000000_r, 0.700000_r, 0.000000_r, 0.170000_r, 0.130000_r, 0.000000_r, 0.000000_r,
    0.570000_r, -0.600000_r, 0.170000_r, 0.000000_r, -0.600000_r, 0.000000_r, 0.285805_r, 0.000000_r, -0.958288_r,
    0.130000_r, 0.600000_r, -0.000000_r, 0.700000_r, 0.600000_r, 0.750000_r,
    0.530000_r, 0.600000_r, 0.750000_r, 0.700000_r, 0.600000_r, 0.170000_r, 0.130000_r, 0.600000_r, -0.000000_r,
    0.170000_r, 0.000000_r, -0.580000_r, -0.400000_r, 0.000000_r, -0.750000_r, 0.000000_r, 1.000000_r, 0.000000_r,
    -0.050000_r, 0.000000_r, -0.000000_r, 0.130000_r, 0.600000_r, 0.570000_r,
    -0.050000_r, 0.600000_r, 0.570000_r, 0.130000_r, 0.600000_r, -0.000000_r, 0.130000_r, 0.000000_r, 0.000000_r,
    0.180000_r, 0.000000_r, -0.570000_r, 0.180000_r, -0.600000_r, -0.570000_r, -0.953583_r, 0.000000_r, -0.301131_r,
    -0.050000_r, 0.000000_r, 0.570000_r, 0.530000_r, 0.600000_r, 0.750000_r,
    0.530000_r, 0.600000_r, 0.750000_r, -0.050000_r, 0.600000_r, 0.570000_r, -0.050000_r, 0.000000_r, 0.570000_r,
    -0.580000_r, 0.000000_r, -0.180000_r, -0.580000_r, -0.600000_r, -0.180000_r, -0.296399_r, 0.000000_r, 0.955064_r,
    0.530000_r, 0.000000_r, 0.170000_r, 0.700000_r, 0.600000_r, 0.750000_r,
    0.700000_r, 0.600000_r, 0.170000_r, 0.530000_r, 0.600000_r, 0.750000_r, 0.530000_r, 0.000000_r, 0.750000_r,
    -0.170000_r, 0.000000_r, 0.580000_r, -0.170000_r, -0.600000_r, 0.580000_r, 0.959629_r, 0.000000_r, 0.281270_r,
    0.130000_r, 0.000000_r, -0.000000_r, 0.700000_r, 0.600000_r, 0.170000_r,
    0.130000_r, 0.600000_r, -0.000000_r, 0.700000_r, 0.600000_r, 0.170000_r, 0.700000_r, 0.000000_r, 0.170000_r,
    0.570000_r, 0.000000_r, 0.170000_r, 0.570000_r, -0.600000_r, 0.170000_r, 0.285805_r, 0.000000_r, -0.958288_r,
};

struct triangle_t
{
    struct vec3_t bbmin, bbmax;
    struct vec3_t v0, v1, v2;
    struct vec3_t E1, E2, normal;
};

struct intersection_t
{
    int8_t i;
    real_t t;
};
static struct intersection_t intersection;

struct ray_t
{
    struct vec3_t start;
    struct vec3_t direction;
    struct vec3_t inv_direction;
};
static struct ray_t ray;

struct interaction_t
{
    int8_t i;
    real_t t;
    const struct triangle_t* surface;
    struct vec3_t position;
    struct vec3_t normal;
};
static struct interaction_t interaction;

#undef TWO_PI
#define INV_PI 0.318310_r
#define TWO_PI 6.283185_r
#define EPSILON 0.000001_r

#define rand01() ((real_t)rand() / RAND_MAX)

static void makeInteraction(void)
{
    interaction.i = intersection.i;
    interaction.t = intersection.t;
    interaction.surface = &((const struct triangle_t*)triangles)[intersection.i];
    vec3_mad_s(interaction.position, ray.start, ray.direction, intersection.t);
    vec3_assign(interaction.normal, interaction.surface->normal);

    if (dot_m(ray.direction, interaction.normal) < 0.0_r)
    {
        vec3_minus(interaction.normal, interaction.normal);
    }

    orthonormalBasis(interaction.normal);
}

static struct vec3_t bbmin;
static struct vec3_t bbmax;

static real_t t;
static struct vec3_t t0;
static struct vec3_t t1;
static struct vec3_t temp;
static uint8_t bb_intersect(void)
{
    vec3_sub(t0, bbmin, ray.start);
    vec3_mul_assign(t0, ray.inv_direction);

    vec3_sub(t1, bbmax, ray.start);
    vec3_mul_assign(t1, ray.inv_direction);

    vec3_min(temp, t0, t1);
    t = std::max(vec3_compMax(temp), 0.0_r);

    vec3_max(temp, t0, t1);
    return vec3_compMin(temp) >= t ? 1 : 0;
}

static const struct triangle_t* s;
static uint8_t tt_intersect(void)
{
    struct vec3_t P;
    cross(P, ray.direction, s->E2);
    const real_t determinant = dot(P, s->E1);
    if (determinant < EPSILON && determinant > -EPSILON)
    {
        return 0;
    }

    const real_t inv_determinant = 1.0_r / determinant;

    struct vec3_t T;
    vec3_sub(T, ray.start, s->v0);
    const real_t u = dot(P, T) * inv_determinant;
    if (u > 1.0_r || u < 0.0_r)
    {
        return 0;
    }

    struct vec3_t Q;
    cross(Q, T, s->E1);
    const real_t v = dot(Q, ray.direction) * inv_determinant;
    if (v > 1.0_r || v < 0.0_r || u + v > 1.0_r)
    {
        return 0;
    }

    t = dot(Q, s->E2) * inv_determinant;
    if (t <= 0.0_r)
    {
        return 0;
    }

    return 1;
}

static uint8_t tb_intersect(void)
{
    vec3_sub(t0, s->bbmin, ray.start);
    vec3_mul_assign(t0, ray.inv_direction);

    vec3_sub(t1, s->bbmax, ray.start);
    vec3_mul_assign(t1, ray.inv_direction);

    vec3_min(temp, t0, t1);
    t = std::max(vec3_compMax(temp), 0.0_r);

    vec3_max(temp, t0, t1);
    return vec3_compMin(temp) >= t ? 1 : 0;
}

static void intersect(void)
{
    intersection.t = std::numeric_limits<real_t>::max();
    intersection.i = -1;

    if (bb_intersect())
    {
        for (size_t k = 0; k < 32; ++k)
        {
            s = &((const struct triangle_t*)triangles)[k];
            if (!tb_intersect())
                continue;
            if (tt_intersect())
            {
                if (t < intersection.t)
                {
                    intersection.t = t;
                    intersection.i = k;
                }
            }
        }
    }
}

static struct vec3_t reflectance;
static void Reflectance(int8_t i)
{
    if (i == 8 || i == 9)
    {
        vec3_assign_s3(reflectance, 0.05_r, 0.65_r, 0.05_r);
    }
    else if (i == 10 || i == 11)
    {
        vec3_assign_s3(reflectance, 0.65_r, 0.05_r, 0.05_r);
    }
    else
    {
        vec3_assign_s(reflectance, 0.65_r);
    }
}

#define abs(x) ((x) > 0 ? x : -x)
static struct vec3_t bsdf_absIdotN;
static real_t bsdf_pdf;
static uint8_t sampleBSDF(void)
{
    struct vec3_t wi;
    struct vec3_t z;
    vec3_assign_s3(z, T.m[2][0], T.m[2][1], T.m[2][2]);
    wi.z = dot(z, ray.direction);

    if (wi.z <= 0.0_r)
    {
        return 0;
    }

    bsdf_pdf = wi.z * INV_PI;

    const real_t temp = INV_PI * abs(wi.z);
    Reflectance(interaction.i);
    vec3_mul_s(bsdf_absIdotN, reflectance, temp);

    return bsdf_pdf > 0.0_r ? 1 : 0;
}

static real_t u0;
static real_t u1;
static real_t u2;

static struct vec3_t linear_r;
static struct vec3_t linear_x;
static struct vec3_t linear_y;
static struct vec3_t linear_z;
static struct vec3_t linear_t;
static void linearCombination(void)
{
    linear_r.x = dot(linear_x, linear_t);
    linear_r.y = dot(linear_y, linear_t);
    linear_r.z = dot(linear_z, linear_t);
}
static int8_t lightIdx;
static struct vec3_t light_pos;
static void lightPoint(void)
{
    const real_t su = sqrt(u0);
    const real_t x = (1 - su);
    const real_t y = (1 - u1) * su;
    const real_t z = u1 * su;
    const struct triangle_t* light = &((const struct triangle_t*)triangles)[lightIdx];
    vec3_assign_s3(linear_t, x, y, z);
    vec3_assign_s3(linear_x, light->v0.x, light->v1.x, light->v2.x);
    vec3_assign_s3(linear_y, light->v0.y, light->v1.y, light->v2.y);
    vec3_assign_s3(linear_z, light->v0.z, light->v1.z, light->v2.z);
    linearCombination();
    vec3_assign(light_pos, linear_r);
}

static void cosWeightedHemi(void)
{
    u0 = rand01();
    u1 = rand01();

    const real_t r = sqrt(u0);
    const real_t azimuth = u1 * TWO_PI;

    struct vec3_t v;
    const auto [sin_a, cos_a] = sincos(azimuth);
    vec3_assign_s3(v, r * cos_a, r * sin_a, sqrt(1.0_r - u0));

    vec3_mad_s(ray.start, interaction.position, interaction.normal, EPSILON);

    struct vec3_t x;
    struct vec3_t y;
    struct vec3_t z;
    vec3_assign_s3(x, T.m[0][0], T.m[1][0], T.m[2][0]);
    vec3_assign_s3(y, T.m[0][1], T.m[1][1], T.m[2][1]);
    vec3_assign_s3(z, T.m[0][2], T.m[1][2], T.m[2][2]);
    ray.direction.x = dot(x, v);
    ray.direction.y = dot(y, v);
    ray.direction.z = dot(z, v);

    vec3_rcp(ray.inv_direction, ray.direction);
}

#define balanceHeuristic(a, b) ((a) / ((a) + (b)))
static struct vec3_t radiance;
static struct vec3_t lightColor;
#define light_area 0.0893_r
static void sampleLight(void)
{
    vec3_assign_s(radiance, 0.0_r);

    u0 = rand01();
    u1 = rand01();
    u2 = rand01();

    lightIdx = u0 < 0.5_r ? 0 : 1;

    lightPoint();
    vec3_mad_s(ray.start, interaction.position, interaction.normal, EPSILON);
    vec3_sub(ray.direction, light_pos, ray.start);
    normalize(ray.direction);
    vec3_rcp(ray.inv_direction, ray.direction);

    const struct triangle_t* light = &((const struct triangle_t*)triangles)[lightIdx];
    const real_t cos_light_theta = dot_m(ray.direction, light->normal);
    if (cos_light_theta <= 0.0_r)
    {
        return;
    }

    const real_t cos_theta = dot(ray.direction, interaction.normal);
    if (cos_theta <= 0.0_r)
    {
        return;
    }

    intersect();
    if (intersection.i == -1 || intersection.i != lightIdx)
    {
        return;
    }

    if (!sampleBSDF())
    {
        return;
    }

    const real_t light_pdf = (intersection.t * intersection.t) / (light_area * cos_light_theta);
    const real_t mis_weight = balanceHeuristic(light_pdf, bsdf_pdf);
    vec3_mul(radiance, bsdf_absIdotN, lightColor);
    vec3_mul_assign_s(radiance, mis_weight);
    vec3_div_assign_s(radiance, light_pdf);
    vec3_div_assign_s(radiance, 0.5_r);
}
static uint16_t depth;
static struct vec3_t sample;
static void sampleRay(void)
{
    struct vec3_t throughput;
    vec3_assign_s(throughput, 1.0_r);

    depth = 0;
    while (1)
    {
        intersect();
        if (intersection.i == -1)
        {
            vec3_add_assign(sample, throughput);
            return;
        }

        if (intersection.i < 2)
        {
            if (depth == 0)
            {
                vec3_add_assign(sample, lightColor);
            }
            return;
        }

        makeInteraction();

        sampleLight();
        vec3_mad_assign(sample, radiance, throughput);

        depth++;
        cosWeightedHemi();
        if (!sampleBSDF())
        {
            return;
        }

        vec3_mul_assign(throughput, bsdf_absIdotN);
        vec3_div_assign_s(throughput, bsdf_pdf);

        if (depth == max_depth)
        {
            return;
        }
    }
}

static struct vec3_t eye;
static struct vec3_t center;
static struct vec3_t up;
static struct vec3_t view_x;
static struct vec3_t view_y;
static struct vec3_t view_z;
static struct mat4_t view;
void precompute_rt(void)
{
    vec3_assign_s3(eye, 0.0_r, 1.0_r, 3.5_r);
    vec3_assign_s3(center, 0.0_r, 1.0_r, 0.0_r);
    vec3_assign_s3(up, 0.0_r, 1.0_r, 0.0_r);

    lookat(view, eye, center, up);

    vec3_assign_s3(view_x, view.m[0][0], view.m[0][1], view.m[0][2]);
    vec3_assign_s3(view_y, view.m[1][0], view.m[1][1], view.m[1][2]);
    vec3_assign_s3(view_z, view.m[2][0], view.m[2][1], view.m[2][2]);

    vec3_assign_s3(lightColor, 200.0_r, 200.0_r, 200.0_r);
    vec3_assign_s3(bbmin, -1.0_r, 0.0_r, -1.0_r);
    vec3_assign_s3(bbmax, 1.0_r, 2.0_r, 1.0_r);
    // X = 0;
    // Y = 0;
}

static constexpr size_t LCD_W = 160;
static constexpr size_t LCD_H = 80;

static constexpr size_t alpha = 45;

static void samplePixel(const uint X, const uint Y)
{
    vec3_assign_s(sample, 0.0_r);
    for (size_t i = 0; i < spp; i++)
    {
        u0 = rand01();
        u1 = rand01();

        const real_t Xw = X + u0;
        const real_t Yw = LCD_H - 1 - Y + u1;
        const real_t Xc = Xw - LCD_W * 0.5_r;
        const real_t Yc = Yw - LCD_H * 0.5_r;
        const real_t Zc = -LCD_H * 0.5_r / tanf(radians(alpha) * 0.5_r);

        vec3_assign_s3(linear_t, Xc, Yc, Zc);
        vec3_assign_s3(linear_x, view_x.x, view_y.x, view_z.x);
        vec3_assign_s3(linear_y, view_x.y, view_y.y, view_z.y);
        vec3_assign_s3(linear_z, view_x.z, view_y.z, view_z.z);
        linearCombination();
        vec3_assign(ray.direction, linear_r);

        vec3_assign(ray.start, eye);
        normalize(ray.direction);
        vec3_rcp(ray.inv_direction, ray.direction);

        sampleRay();
    }

    vec3_div_assign_s(sample, spp);

    struct vec3_t one;
    vec3_assign_s(one, 1.0_r);

    struct vec3_t sample_one;
    vec3_add(sample_one, sample, one);

    vec3_div_assign(sample, sample_one);
}

// template<typename ...Ts>
void drawpixel(const uint x, const uint y, const uint16_t color){

}
static RGB565 draw3drt(const uint X, const uint Y){
    samplePixel(X, Y);
    // const uint16_t r = uint16_t(sample.x * 0.1_r);
    // const uint16_t g = uint16_t(sample.y * 0.3_r);
    // const uint16_t b = uint16_t(sample.z * 0.1_r);
    // const uint16_t c16 = (r << 11) | (g << 5) | b;
    return RGB565(uint8_t(sample.x * 255), uint8_t(sample.y * 255), uint8_t(sample.z * 255));
}

static void render_row(const std::span<RGB565> row, const uint y){
    // ASSERT(row.size() == LCD_W);

    for (uint x = 0; x < LCD_W; x++){
        row[x] = draw3drt(x, y);
    }
}

#define UART hal::uart2
using drivers::ST7789;

void light_tracking_main(void){

    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.noBrackets();

    DEBUG_PRINTLN(micros());

    #ifdef CH32V30X
    auto & spi = spi2;
    auto & lcd_blk = portC[7];
    
    lcd_blk.outpp(HIGH);

    auto & lcd_cs = portD[6];
    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];
    #else
    auto & spi = spi1;
    auto & lcd_blk = portA[10];
    auto & lcd_cs = portA[15];
    auto & lcd_dc = portA[11];
    auto & dev_rst = portA[12];
    
    
    lcd_blk.outpp(HIGH);
    #endif

    


    spi.bind_cs_pin(lcd_cs, 0);
    spi.init(144_MHz, CommStrategy::Blocking);
    // spi.init(36_MHz, CommStrategy::Blocking, CommStrategy::None);

    // ST7789 tftDisplayer({{spi, 0}, lcd_dc, dev_rst}, {240, 134});
    ST7789 tftDisplayer({{spi, 0}, lcd_dc, dev_rst}, {240, 135});

    {
        tftDisplayer.init();


        if(true ){
        // if(false){
            tftDisplayer.set_flip_x(false);
            tftDisplayer.set_flip_y(true);
            tftDisplayer.set_swap_xy(true);
            tftDisplayer.set_display_offset({40, 52}); 
        }else{
            tftDisplayer.set_flip_x(true);
            tftDisplayer.set_flip_y(true);
            tftDisplayer.set_swap_xy(false);
            tftDisplayer.set_display_offset({52, 40}); 
        }
        tftDisplayer.set_format_rgb(true);
        tftDisplayer.set_flush_dir_h(false);
        tftDisplayer.set_flush_dir_v(false);
        tftDisplayer.set_inversion(true);
    }

    tftDisplayer.fill(ColorEnum::BLACK);

	precompute_rt();

    for (uint y = 0; y < LCD_H; y++){
        std::array<RGB565, LCD_W> row;
        render_row(row, y);
        // DEBUG_PRINTLN(std::span(reinterpret_cast<const uint16_t * >(row.data()), row.size()));
        tftDisplayer.put_texture(Rect2i(Vector2i(0,y), Vector2i(LCD_W, 1)), row.data());
        // tftDisplayer.put_rect(Rect2i(Vector2i(0,y), Vector2i(LCD_W, 1)), ColorEnum::WHITE);
        // renderer.draw_rect(Rect2i(20, 0, 20, 40));
    }

    DEBUG_PRINTLN(micros());
}
