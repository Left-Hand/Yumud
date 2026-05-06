#include "../decimal.hpp"
#include "../fixedpoint.hpp"
#include "../nondecimal.hpp"
#include "../scientific_fixedpoint.hpp"
#include "../fmtnum.hpp"
#include "core/utils/Result.hpp"

using namespace ymd;
using namespace ymd::str;


namespace {

static_assert(depart_abs_fixedpoint((114514) << 10, 4, 10).digit_part == 114514);
static_assert(depart_abs_fixedpoint((114514) << 10, 4, 10).frac_part == 0);

static_assert(depart_abs_fixedpoint((114514) << 1, 4, 1).digit_part == 114514);
static_assert(depart_abs_fixedpoint((114514) << 1, 4, 1).frac_part == 0);

static_assert(depart_abs_fixedpoint(0xffff0000, 4, 32).digit_part == 1);
static_assert(depart_abs_fixedpoint(0xffff0000, 4, 32).frac_part == 0);

static_assert(depart_abs_f32(114.5140, 4).digit_part == 114);
static_assert(depart_abs_f32(114.5140, 4).frac_part == 5140);

static_assert(depart_abs_f32(123.456, 3).digit_part == 123);
static_assert(depart_abs_f32(123.456, 3).frac_part == 456);

static_assert(depart_abs_f32(1099.999, 3).digit_part == 1099);
static_assert(depart_abs_f32(1099.999, 3).frac_part == 999);

static_assert(depart_abs_f32(1099.9994, 3).digit_part == 1099);
static_assert(depart_abs_f32(1099.9994, 3).frac_part == 999);

static_assert(depart_abs_f32(1099.9996, 3).digit_part == 1100);
static_assert(depart_abs_f32(1099.9996, 3).frac_part == 000);

static_assert(depart_abs_f32(1E-8, 3).frac_part == 000);

static_assert(depart_abs_f32(1E6, 3).digit_part == 1E6);
static_assert(depart_abs_f32(1E6, 3).frac_part == 000);

static_assert(depart_abs_f32(1E7, 3).digit_part == 1E7);
static_assert(depart_abs_f32(1E7, 3).frac_part == 000);

static_assert(depart_abs_f32(1E7, 5).digit_part == 1E7);
static_assert(depart_abs_f32(1E7, 5).frac_part == 000);


#if 0
static_assert(depart_abs_f32(1E10, 3).digit_part == 1E10);
static_assert(depart_abs_f32(1E10, 3).frac_part == 000);
#endif

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.99993) * (1u << 24)), 3, 24).digit_minor_number == 9);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.99993) * (1u << 24)), 3, 24).frac_part == 999);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.99993) * (1u << 24)), 3, 24).exponent == -1);


static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.99997) * (1u << 24)), 3, 24).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.99997) * (1u << 24)), 3, 24).frac_part == 0);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.99997) * (1u << 24)), 3, 24).exponent == 0);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.59993) * (1u << 24)), 3, 24).digit_minor_number == 5);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.59993) * (1u << 24)), 3, 24).frac_part == 999);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.59993) * (1u << 24)), 3, 24).exponent == -1);


static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.59997) * (1u << 24)), 3, 24).digit_minor_number == 6);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.59997) * (1u << 24)), 3, 24).frac_part == 0);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.59997) * (1u << 24)), 3, 24).exponent == -1);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.099999997) * (1u << 24)), 3, 24).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.099999997) * (1u << 24)), 3, 24).frac_part == 0);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.099999997) * (1u << 24)), 3, 24).exponent == -1);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((9.9999997) * (1u << 24)), 3, 24).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((9.9999997) * (1u << 24)), 3, 24).frac_part == 0);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((9.9999997) * (1u << 24)), 3, 24).exponent == 1);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.125) * (1u << 16)), 4, 16).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.125) * (1u << 16)), 4, 16).frac_part == 2500);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.125) * (1u << 16)), 4, 16).exponent == -1);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((1.125) * (1u << 16)), 4, 16).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((1.125) * (1u << 16)), 4, 16).frac_part == 1250);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((1.125) * (1u << 16)), 4, 16).exponent == 0);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((919.514) * (1u << 16)), 4, 16).digit_minor_number == 9);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((919.514) * (1u << 16)), 4, 16).frac_part == 1951);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((919.514) * (1u << 16)), 4, 16).exponent == 2);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((11451) * (1u << 2)), 4, 2).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((11451) * (1u << 2)), 4, 2).frac_part == 1451);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((11451) * (1u << 2)), 4, 2).exponent == 4);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((114.51) * (1u << 16)), 4, 16).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((114.51) * (1u << 16)), 4, 16).frac_part == 1451);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((114.51) * (1u << 16)), 4, 16).exponent == 2);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.01145) * (1u << 24)), 5, 24).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.01145) * (1u << 24)), 5, 24).frac_part == 14500);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.01145) * (1u << 24)), 5, 24).exponent == -2);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.01145) * (1ull << 32)), 4, 32).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.01145) * (1ull << 32)), 4, 32).frac_part == 1450);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.01145) * (1ull << 32)), 4, 32).exponent == -2);


static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.001919) * (1u << 24)), 4, 24).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.001919) * (1u << 24)), 4, 24).frac_part == 9190);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.001919) * (1u << 24)), 4, 24).exponent == -3);


static_assert(depart_abs_fixedpoint_scientific((uint32_t)((1.9999995) * (1u << 24)), 4, 24).digit_minor_number == 2);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((1.9999995) * (1u << 24)), 4, 24).frac_part == 0);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((1.9999995) * (1u << 24)), 4, 24).exponent == 0);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.9999995) * (1u << 24)), 3, 24).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.9999995) * (1u << 24)), 3, 24).frac_part == 0);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.9999995) * (1u << 24)), 3, 24).exponent == 0);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((9.9999995) * (1u << 24)), 3, 24).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((9.9999995) * (1u << 24)), 3, 24).frac_part == 0);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((9.9999995) * (1u << 24)), 3, 24).exponent == 1);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((2.5555555) * (1u << 24)), 3, 24).digit_minor_number == 2);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((2.5555555) * (1u << 24)), 3, 24).frac_part == 556);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.00999995) * (1u << 24)), 3, 24).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.00999995) * (1u << 24)), 3, 24).frac_part == 0);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.00999995) * (1u << 24)), 3, 24).exponent == -2);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((11451.9995) * (1u << 2)), 4, 2).digit_minor_number == 1);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((11451.9995) * (1u << 2)), 4, 2).frac_part == 1452);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((11451.9995) * (1u << 2)), 4, 2).exponent == 4);

static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.5999995) * (1u << 24)), 3, 24).digit_minor_number == 6);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.5999995) * (1u << 24)), 3, 24).frac_part == 0);
static_assert(depart_abs_fixedpoint_scientific((uint32_t)((0.5999995) * (1u << 24)), 3, 24).exponent == -1);


static_assert(frac_abs_f32_nonfpu(114.514f, 10000) == 5140);
static_assert(frac_abs_f32_nonfpu(0.1919f, 10000) == 1919);
static_assert(frac_abs_f32_nonfpu(0.19185f, 10000) == 1919);
static_assert(frac_abs_f32_nonfpu(0.191849f, 10000) == 1918);
static_assert(frac_abs_f32_nonfpu(0.114514f, 1000000) == 114514);
static_assert(frac_abs_f32_nonfpu(0.001f, 1000) == 1);
static_assert(frac_abs_f32_nonfpu(1e-8f, 1000) == 0);


static_assert(frac_abs_f32_nonfpu(1099.9996, 1e3) == 1000);

static_assert(floor_abs_f32_nonfpu(1000) == 1000);
static_assert(floor_abs_f32_nonfpu(1e6) == 1e6);
static_assert(floor_abs_f32_nonfpu(1e7) == 1e7);
// static_assert(floor_abs_f32_nonfpu(1e8) == 1e8);
// static_assert(floor_abs_f32(1e8) == 1e8);
}

