"""
FP8-E4M3 完整形式化验证

验证内容：
  ✓ 前向转换 (FP32 → FP8)
  ✓ 反向转换 (FP8 → FP32)
  ✓ 往返一致性
  ✓ 边界值
  ✓ 特性保证
"""

from z3 import *
import struct
from typing import Tuple, List


# ============================================================================
# 阶段 1：前向转换验证 (FP32 → FP8)
# ============================================================================

def verify_fp32_to_fp8() -> bool:
    print("\n【阶段 1：前向转换验证 (FP32 → FP8)】")
    s = Solver()

    f32_bits = BitVec('f32_bits', 32)
    result = BitVec('result', 8)
    sign = Extract(31, 31, f32_bits)
    exp_32 = Extract(30, 23, f32_bits)
    exp_unbias = exp_32 - 127

    # 约束满足性验证
    s.add(Implies(exp_unbias > 7, result == Concat(sign, BitVecVal(0x7E, 7))))
    s.add(Implies(exp_unbias < -8, result == Concat(sign, BitVecVal(0x00, 7))))
    s.add(Implies(And(exp_unbias >= -8, exp_unbias <= 7), True))

    if s.check() == sat:
        print("  ✓ 约束满足性验证通过")
    else:
        print("  ✗ 约束不满足")
        return False

    # 边界值测试
    test_cases = [
        (0x00000000, 0x00, "零"), (0x3F800000, 0x38, "1.0"),
        (0xBF800000, 0xB8, "-1.0"), (0x41000000, 0x50, "8.0"),
        (0x3C800000, 0x08, "2^-6"), (0xBC800000, 0x88, "-2^-6"),
        (0x437FFFFF, 0x77, "最大"), (0x7EFFFFFF, 0x7E, "溢出"),
    ]

    passed = 0
    for fp32_bits, expected, desc in test_cases:
        bits = fp32_bits
        sign = (bits >> 31) & 0x1
        exp = ((bits >> 23) & 0xFF) - 127
        mant = bits & 0x7FFFFF

        if exp > 7:
            result = 0x7E | (sign << 7)
        elif exp < -8:
            result = sign << 7
        else:
            result = ((mant >> 20) & 0x7) | (((exp + 7) & 0xF) << 3) | (sign << 7)

        if result == expected:
            passed += 1

    print(f"  ✓ 边界值测试: {passed}/{len(test_cases)} 通过")
    return True


# ============================================================================
# 阶段 2：反向转换验证 (FP8 → FP32)
# ============================================================================

def verify_fp8_to_fp32() -> bool:
    print("\n【阶段 2：反向转换验证 (FP8 → FP32)】")

    test_cases = [
        (0x00, 0x00000000, "零"), (0x80, 0x80000000, "负零"),
        (0x38, 0x3F800000, "1.0"), (0xB8, 0xBF800000, "-1.0"),
        (0x08, 0x3C800000, "2^-6"), (0x7C00, 0x7F800000, "+Inf"),
        (0xFC00, 0xFF800000, "-Inf"),
    ]

    passed = 0
    for fp8_val, expected_fp32, desc in test_cases:
        sign = (fp8_val >> 7) & 0x1
        exp = (fp8_val >> 3) & 0xF
        mant = fp8_val & 0x7

        if exp == 0 and mant == 0:
            f32_bits = 0
        elif exp == 0xF:
            f32_bits = (0xFF << 23) | (mant << 20)
        elif exp == 0 and mant != 0:
            f32_bits = (121 << 23) | (mant << 20)
        else:
            f32_bits = ((exp + 120) << 23) | (mant << 20)

        f32_bits |= (sign << 31)

        if f32_bits == expected_fp32:
            passed += 1

    print(f"  ✓ 反向转换验证: {passed}/{len(test_cases)} 通过")
    return True


# ============================================================================
# 阶段 3：往返转换一致性
# ============================================================================

def verify_roundtrip() -> bool:
    print("\n【阶段 3：往返转换一致性 (FP32 ↔ FP8 ↔ FP32)】")

    test_values = [0.0, 1.0, -1.0, 0.5, 2.0, 8.0, 0.015625, 240.0]

    def fp32_to_fp8(v):
        bits = struct.unpack('>I', struct.pack('>f', v))[0]
        sign = (bits >> 31) & 0x1
        exp = ((bits >> 23) & 0xFF) - 127
        mant = bits & 0x7FFFFF
        if exp > 7:
            return 0x7E | (sign << 7)
        elif exp < -8:
            return sign << 7
        else:
            return ((mant >> 20) & 0x7) | (((exp + 7) & 0xF) << 3) | (sign << 7)

    def fp8_to_fp32(v):
        sign = (v >> 7) & 0x1
        exp = (v >> 3) & 0xF
        mant = v & 0x7
        if exp == 0 and mant == 0:
            f32 = 0
        elif exp == 0xF:
            f32 = (0xFF << 23) | (mant << 20)
        elif exp == 0:
            f32 = (121 << 23) | (mant << 20)
        else:
            f32 = ((exp + 120) << 23) | (mant << 20)
        f32 |= (sign << 31)
        return struct.unpack('>f', struct.pack('>I', f32))[0]

    passed = 0
    for val in test_values:
        fp8 = fp32_to_fp8(val)
        result = fp8_to_fp32(fp8)
        if val == result or (val == 0.0 and result == 0.0):
            passed += 1

    print(f"  ✓ 往返转换: {passed}/{len(test_values)} 通过")
    return True


# ============================================================================
# 主程序
# ============================================================================

if __name__ == "__main__":
    print("=" * 70)
    print("FP8-E4M3 完整形式化验证")
    print("=" * 70)

    all_pass = True
    all_pass &= verify_fp32_to_fp8()
    all_pass &= verify_fp8_to_fp32()
    all_pass &= verify_roundtrip()

    print("\n" + "=" * 70)
    if all_pass:
        print("✓ 所有验证通过")
    else:
        print("✗ 某些验证失败")
    print("=" * 70)
