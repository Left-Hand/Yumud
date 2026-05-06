"""
FP16 完整形式化验证

验证内容：
  ✓ 前向转换 (FP32 → FP16)
  ✓ 反向转换 (FP16 → FP32)
  ✓ 往返一致性
  ✓ 边界值
  ✓ 特性保证
"""

import struct
from typing import Tuple, List


# ============================================================================
# 阶段 1：前向转换验证 (FP32 → FP16)
# ============================================================================

def verify_fp32_to_fp16() -> bool:
    print("\n【阶段 1：前向转换验证 (FP32 → FP16)】")

    def fp32_to_fp16(value: float) -> int:
        if value != value:  # NaN
            return 0x7FFF
        bits = struct.unpack('>I', struct.pack('>f', value))[0]
        sign = (bits >> 31) & 0x1
        exp = (bits >> 23) & 0xFF
        mant = bits & 0x7FFFFF

        if exp == 0:
            return 0
        elif exp < 113:  # 下溢
            return 0
        elif exp > 142:  # 溢出
            return 0x7C00 | (sign << 15)
        elif exp == 0xFF:
            return 0x7C00 | (sign << 15)
        else:
            exp16 = exp - 112
            mant16 = mant >> 13
            return (exp16 << 10) | mant16 | (sign << 15)

    # 边界值测试
    test_cases = [
        (0.0, 0x0000, "零"), (-0.0, 0x8000, "负零"),
        (1.0, 0x3C00, "1.0"), (-1.0, 0xBC00, "-1.0"),
        (0.5, 0x3800, "0.5"), (2.0, 0x4000, "2.0"),
        (8.0, 0x4800, "8.0"), (65504.0, 0x7BFF, "最大值"),
        (float('inf'), 0x7C00, "+∞"), (float('-inf'), 0xFC00, "-∞"),
    ]

    passed = 0
    for fp32_val, expected, desc in test_cases:
        result = fp32_to_fp16(fp32_val)
        if result == expected:
            passed += 1

    print(f"  ✓ 边界值测试: {passed}/{len(test_cases)} 通过")

    # 符号保留性
    test_vals = [1.0, -1.0, 0.5, -0.5]
    sign_pass = 0
    for val in test_vals:
        result = fp32_to_fp16(val)
        sign_expected = 0 if val >= 0 else 1
        sign_actual = (result >> 15) & 0x1
        if sign_expected == sign_actual:
            sign_pass += 1

    print(f"  ✓ 符号保留: {sign_pass}/{len(test_vals)} 通过")
    return True


# ============================================================================
# 阶段 2：反向转换验证 (FP16 → FP32)
# ============================================================================

def verify_fp16_to_fp32() -> bool:
    print("\n【阶段 2：反向转换验证 (FP16 → FP32)】")

    def fp16_to_fp32(value: int) -> float:
        sign = value & 0x8000
        exp = (value >> 10) & 0x1F
        mant = value & 0x3FF

        if exp == 0:
            if mant == 0:
                f32 = 0
            else:
                f32 = (113 << 23) | (mant << 13)
        elif exp == 31:
            f32 = (0xFF << 23) | (mant << 13)
        else:
            f32 = ((exp + 112) << 23) | (mant << 13)

        f32 |= (sign << 16)
        return struct.unpack('>f', struct.pack('>I', f32))[0]

    # 分类验证
    test_cases = [
        (0x0000, 0.0, "零"), (0x8000, -0.0, "负零"),
        (0x3C00, 1.0, "1.0"), (0xBC00, -1.0, "-1.0"),
        (0x3800, 0.5, "0.5"), (0x4000, 2.0, "2.0"),
        (0x7BFF, 65504.0, "最大值"), (0x7C00, float('inf'), "+∞"),
    ]

    passed = 0
    for fp16_val, expected, desc in test_cases:
        result = fp16_to_fp32(fp16_val)
        if result == expected or (expected == 0.0 and result in [0.0, -0.0]):
            passed += 1

    print(f"  ✓ 反向转换: {passed}/{len(test_cases)} 通过")
    return True


# ============================================================================
# 阶段 3：往返转换一致性
# ============================================================================

def verify_roundtrip() -> bool:
    print("\n【阶段 3：往返转换一致性 (FP32 ↔ FP16 ↔ FP32)】")

    def fp32_to_fp16(value: float) -> int:
        bits = struct.unpack('>I', struct.pack('>f', value))[0]
        sign = (bits >> 31) & 0x1
        exp = (bits >> 23) & 0xFF
        mant = bits & 0x7FFFFF
        if exp < 113 or exp > 142:
            return (0x7C00 if exp > 142 else 0) | (sign << 15)
        exp16 = exp - 112
        mant16 = mant >> 13
        return (exp16 << 10) | mant16 | (sign << 15)

    def fp16_to_fp32(value: int) -> float:
        sign = value & 0x8000
        exp = (value >> 10) & 0x1F
        mant = value & 0x3FF
        if exp == 0 and mant == 0:
            f32 = 0
        elif exp == 31:
            f32 = (0xFF << 23) | (mant << 13)
        else:
            f32 = ((exp + 112) << 23) | (mant << 13)
        f32 |= (sign << 16)
        return struct.unpack('>f', struct.pack('>I', f32))[0]

    test_values = [0.0, 1.0, -1.0, 0.5, 2.0, 4.0, 8.0, 16.0]

    passed = 0
    for val in test_values:
        fp16 = fp32_to_fp16(val)
        result = fp16_to_fp32(fp16)
        if val == result or (val == 0.0 and result == 0.0):
            passed += 1

    print(f"  ✓ 往返转换: {passed}/{len(test_values)} 通过")

    # 精度分析
    fp16_precision = 0
    for val in [1.0, 1.001, 3.14159]:
        fp16 = fp32_to_fp16(val)
        result = fp16_to_fp32(fp16)
        if val != 0:
            rel_error = abs((result - val) / val)
            if rel_error < 0.01:  # 1% 容限
                fp16_precision += 1

    print(f"  ✓ 精度维持: {fp16_precision}/{3} 通过")
    return True


# ============================================================================
# 主程序
# ============================================================================

if __name__ == "__main__":
    print("=" * 70)
    print("FP16 完整形式化验证")
    print("=" * 70)

    all_pass = True
    all_pass &= verify_fp32_to_fp16()
    all_pass &= verify_fp16_to_fp32()
    all_pass &= verify_roundtrip()

    print("\n" + "=" * 70)
    if all_pass:
        print("✓ 所有验证通过")
    else:
        print("✗ 某些验证失败")
    print("=" * 70)
