#include "../decimal.hpp"
#include "../fixedpoint.hpp"
#include "../nondecimal.hpp"
#include "../scientific_fixedpoint.hpp"
#include "../fmtnum.hpp"
#include "core/utils/Result.hpp"

using namespace ymd;
using namespace ymd::str;


namespace {

static_assert(depart_abs_fixedpoint32((114514) << 10, 4, 10).digit_part == 114514);
static_assert(depart_abs_fixedpoint32((114514) << 10, 4, 10).frac_part == 0);

static_assert(depart_abs_fixedpoint32((114514) << 1, 4, 1).digit_part == 114514);
static_assert(depart_abs_fixedpoint32((114514) << 1, 4, 1).frac_part == 0);

static_assert(depart_abs_fixedpoint32(0xffff0000, 4, 32).digit_part == 1);
static_assert(depart_abs_fixedpoint32(0xffff0000, 4, 32).frac_part == 0);

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


// ==================== 64位定点数拆分测试 ====================

// --- 基本整数测试 (q_num=10, precision=4) ---
static_assert(depart_abs_fixedpoint64((114514ull) << 10, 4, 10).digit_part == 114514);
static_assert(depart_abs_fixedpoint64((114514ull) << 10, 4, 10).frac_part == 0);

// --- q_num=16, precision=4: 114.514 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(114.514 * (1ull << 16)), 4, 16).digit_part == 114);
static_assert(depart_abs_fixedpoint64((uint64_t)(114.514 * (1ull << 16)), 4, 16).frac_part == 5140);

// --- q_num=32, precision=6: π ≈ 3.141592 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(3.141592 * (1ull << 32)), 6, 32).digit_part == 3);
static_assert(depart_abs_fixedpoint64((uint64_t)(3.141592 * (1ull << 32)), 6, 32).frac_part == 141592);

// --- q_num=32, precision=8: 0.12345678 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(0.12345678 * (1ull << 32)), 8, 32).digit_part == 0);
static_assert(depart_abs_fixedpoint64((uint64_t)(0.12345678 * (1ull << 32)), 8, 32).frac_part == 12345678);

// --- q_num=48, precision=5: 1.5 精确值 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(1.5 * (1ull << 48)), 5, 48).digit_part == 1);
static_assert(depart_abs_fixedpoint64((uint64_t)(1.5 * (1ull << 48)), 5, 48).frac_part == 50000);

// --- q_num=48, precision=4: 0.0625 (2^-4) 精确值 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(0.0625 * (1ull << 48)), 4, 48).digit_part == 0);
static_assert(depart_abs_fixedpoint64((uint64_t)(0.0625 * (1ull << 48)), 4, 48).frac_part == 625);

// --- q_num=63 (边界), precision=4: 0.5 ---
static_assert(depart_abs_fixedpoint64(1ull << 62, 4, 63).digit_part == 0);
static_assert(depart_abs_fixedpoint64(1ull << 62, 4, 63).frac_part == 5000);

// --- q_num=63 (边界), precision=4: 0.75 ---
static_assert(depart_abs_fixedpoint64((1ull << 62) | (1ull << 61), 4, 63).digit_part == 0);
static_assert(depart_abs_fixedpoint64((1ull << 62) | (1ull << 61), 4, 63).frac_part == 7500);

// --- 舍入进位测试: q_num=32, precision=3, 0.9996 → 1.000 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(0.9996 * (1ull << 32)), 3, 32).digit_part == 1);
static_assert(depart_abs_fixedpoint64((uint64_t)(0.9996 * (1ull << 32)), 3, 32).frac_part == 0);

// --- 舍入不进位测试: q_num=32, precision=3, 0.9994 → 0.999 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(0.9994 * (1ull << 32)), 3, 32).digit_part == 0);
static_assert(depart_abs_fixedpoint64((uint64_t)(0.9994 * (1ull << 32)), 3, 32).frac_part == 999);

// --- 舍入进位传播: q_num=32, precision=3, 1099.9996 → 1100.000 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(1099.9996 * (1ull << 32)), 3, 32).digit_part == 1100);
static_assert(depart_abs_fixedpoint64((uint64_t)(1099.9996 * (1ull << 32)), 3, 32).frac_part == 0);

// --- 超大整数部分 (>32位): 2^33 = 8589934592, q_num=10, precision=4 ---
static_assert(depart_abs_fixedpoint64(8589934592ull << 10, 4, 10).digit_part == 8589934592ull);
static_assert(depart_abs_fixedpoint64(8589934592ull << 10, 4, 10).frac_part == 0);

// --- 大整数加小数: q_num=16, precision=6 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(100000.25 * (1ull << 16)), 6, 16).digit_part == 100000);
static_assert(depart_abs_fixedpoint64((uint64_t)(100000.25 * (1ull << 16)), 6, 16).frac_part == 250000);

// --- q_num=3, precision=4: 0.375 (0b0.011) ---
static_assert(depart_abs_fixedpoint64(3ull, 4, 3).digit_part == 0);
static_assert(depart_abs_fixedpoint64(3ull, 4, 3).frac_part == 3750);

// --- q_num=3, precision=4: 1.625 (0b1.101) ---
static_assert(depart_abs_fixedpoint64(13ull, 4, 3).digit_part == 1);
static_assert(depart_abs_fixedpoint64(13ull, 4, 3).frac_part == 6250);

// --- q_num=2, precision=4: 1.75 (0b1.11) ---
static_assert(depart_abs_fixedpoint64(7ull, 4, 2).digit_part == 1);
static_assert(depart_abs_fixedpoint64(7ull, 4, 2).frac_part == 7500);

// --- q_num=2, precision=2: 0.25 (0b0.01) ---
static_assert(depart_abs_fixedpoint64(1ull, 2, 2).digit_part == 0);
static_assert(depart_abs_fixedpoint64(1ull, 2, 2).frac_part == 25);

// --- precision=0 舍入: q_num=32, 114.514 → 115 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(114.514 * (1ull << 32)), 0, 32).digit_part == 115);
static_assert(depart_abs_fixedpoint64((uint64_t)(114.514 * (1ull << 32)), 0, 32).frac_part == 0);

// --- precision=0 不舍入: q_num=32, 114.3 → 114 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(114.3 * (1ull << 32)), 0, 32).digit_part == 114);
static_assert(depart_abs_fixedpoint64((uint64_t)(114.3 * (1ull << 32)), 0, 32).frac_part == 0);

// --- q_num=32, precision=8 (最大有效精度): 0.98765433 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(0.98765433 * (1ull << 32)), 8, 32).digit_part == 0);
static_assert(depart_abs_fixedpoint64((uint64_t)(0.98765433 * (1ull << 32)), 8, 32).frac_part == 98765433);

// --- q_num=32, precision=8 舍入进位: 0.999999996 → 1.00000000 ---
// 使用位运算构造接近1.0的值，避免浮点精度不足
static_assert(depart_abs_fixedpoint64(0xFFFFFFFFull, 8, 32).digit_part == 1);
static_assert(depart_abs_fixedpoint64(0xFFFFFFFFull, 8, 32).frac_part == 0);

// --- q_num=64 边界回退为 63: 所有位都为1, 小数部分 ≈ 1-2^-63, 舍入进位到1 ---
static_assert(depart_abs_fixedpoint64(UINT64_MAX, 4, 64).digit_part == 1);
static_assert(depart_abs_fixedpoint64(UINT64_MAX, 4, 64).frac_part == 0);

// --- q_num=60, precision=6: 0.333333 ---
static_assert(depart_abs_fixedpoint64((uint64_t)(0.333333 * (1ull << 60)), 6, 60).digit_part == 0);
static_assert(depart_abs_fixedpoint64((uint64_t)(0.333333 * (1ull << 60)), 6, 60).frac_part == 333333);

}

