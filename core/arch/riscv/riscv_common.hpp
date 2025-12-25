#pragma once

namespace riscv_abilities{
#ifdef __riscv
#ifdef __riscv_float_abi_single
static constexpr bool has_native_hard_f32 = true;
#else
static constexpr bool has_native_hard_f32 = false;
#endif

#ifdef __riscv_float_abi_double
static constexpr bool has_native_hard_f64 = true;
#else
static constexpr bool has_native_hard_f64 = false;
#endif

static constexpr bool has_fpu = has_native_hard_f32 || has_native_hard_f64;

#ifdef __riscv_m
static constexpr bool has_m_extension = true;
#else
static constexpr bool has_m_extension = false;
#endif

#ifdef __riscv_c
static constexpr bool has_c_extension = true;
#else
static constexpr bool has_c_extension = false;
#endif

#ifdef __riscv_a
static constexpr bool has_a_extension = true;
#else
static constexpr bool has_a_extension = false;
#endif

#ifdef __riscv_v
static constexpr bool has_v_extension = true;
#else
static constexpr bool has_v_extension = false;
#endif

#ifdef __riscv_b
static constexpr bool has_b_extension = true;
#else
static constexpr bool has_b_extension = false;
#endif
#endif

}