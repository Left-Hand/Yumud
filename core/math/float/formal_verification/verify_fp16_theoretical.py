"""
FP16 理论形式化验证（使用 Z3）

验证内容：
  ✓ 前向转换 (FP32 → FP16) 的公理正确性
  ✓ 反向转换 (FP16 → FP32) 的公理正确性
  ✓ 往返一致性（理论证明）
  ✓ 特殊值处理（理论证明）
  ✓ 边界条件（理论证明）
"""

from z3 import *

# FP32: 1 sign + 8 exp + 23 mant
# FP16: 1 sign + 5 exp + 10 mant

FP32_BITS = 32
FP16_BITS = 16


def extract_fp32_components(bv):
    """从位向量中提取 FP32 组件"""
    sign = LShR(bv, 31) & 1
    exp  = LShR(bv, 23) & 0xFF
    mant = bv & 0x7FFFFF
    return sign, exp, mant


def extract_fp16_components(bv):
    """从位向量中提取 FP16 组件"""
    sign = LShR(bv, 15) & 1
    exp  = LShR(bv, 10) & 0x1F
    mant = bv & 0x3FF
    return sign, exp, mant


def fp32_to_fp16_symbolic(fp32_bv):
    """FP32 → FP16 转换（符号版本）"""
    sign, exp, mant = extract_fp32_components(fp32_bv)

    # 特殊值处理
    is_nan = (exp == 0xFF) & (mant != 0)
    is_inf_pos = (exp == 0xFF) & (mant == 0) & (sign == 0)
    is_inf_neg = (exp == 0xFF) & (mant == 0) & (sign == 1)
    
    # 下溢：exp < 113 → 零
    is_underflow = ULT(exp, 113)
    # 上溢：exp > 142 → inf
    is_overflow = UGT(exp, 142)

    # 正常数路径
    exp16 = exp - 112
    mant16 = LShR(mant, 13)  # 截断，不考虑舍入

    # 构造 FP16 位
    normal_fp16 = (sign << 15) | (exp16 << 10) | mant16
    zero_fp16 = sign << 15  # ±0
    inf_pos_fp16 = 0x7C00
    inf_neg_fp16 = 0xFC00
    nan_fp16 = 0x7FFF  # canonical NaN

    # 条件选择
    result = If(is_nan, nan_fp16,
                If(is_inf_pos, inf_pos_fp16,
                   If(is_inf_neg, inf_neg_fp16,
                      If(is_underflow, zero_fp16,
                         If(is_overflow, If(sign == 1, inf_neg_fp16, inf_pos_fp16), normal_fp16)))))
    return result


def fp16_to_fp32_symbolic(fp16_bv):
    """FP16 → FP32 转换（符号版本）"""
    sign = LShR(fp16_bv, 15) & 1
    exp16 = LShR(fp16_bv, 10) & 0x1F
    mant16 = fp16_bv & 0x3FF

    # FP32 组件
    sign32 = sign
    
    # 分类处理
    is_zero = (exp16 == 0) & (mant16 == 0)
    is_inf_or_nan = (exp16 == 0x1F)

    # 正常数：exp32 = exp16 + 112, mant32 = mant16 << 13
    exp32_normal = exp16 + 112
    mant32_normal = mant16 << 13

    # 零
    exp32_zero = 0
    mant32_zero = 0

    # Inf/NaN
    exp32_special = 0xFF
    mant32_special = mant16 << 13  # preserve payload

    exp32 = If(is_zero, exp32_zero,
               If(is_inf_or_nan, exp32_special, exp32_normal))
    mant32 = If(is_zero, mant32_zero,
                If(is_inf_or_nan, mant32_special, mant32_normal))

    fp32_bits = (sign32 << 31) | (exp32 << 23) | mant32
    return fp32_bits


def verify_special_values():
    """验证特殊值处理"""
    print("\n【公理验证：特殊值处理】")
    
    s = Solver()

    # 测试 +inf: 0x7F800000
    pos_inf = BitVecVal(0x7F800000, 32)
    fp16_inf = fp32_to_fp16_symbolic(pos_inf)
    s.add(fp16_inf != 0x7C00)
    if s.check() == sat:
        print("  ✗ +inf 映射错误")
        return False
    s.reset()
    print("  ✓ +∞ 映射正确 (0x7C00)")

    # 测试 -inf: 0xFF800000
    neg_inf = BitVecVal(0xFF800000, 32)
    fp16_neg_inf = fp32_to_fp16_symbolic(neg_inf)
    s.add(fp16_neg_inf != 0xFC00)
    if s.check() == sat:
        print("  ✗ -inf 映射错误")
        return False
    s.reset()
    print("  ✓ -∞ 映射正确 (0xFC00)")

    # 测试 NaN: any with exp=0xFF, mant≠0 → 0x7FFF
    nan_val = BitVec('nan_val', 32)
    sign_n, exp_n, mant_n = extract_fp32_components(nan_val)
    s.add(exp_n == 0xFF)
    s.add(mant_n != 0)
    fp16_nan = fp32_to_fp16_symbolic(nan_val)
    s.add(fp16_nan != 0x7FFF)
    if s.check() == sat:
        print("  ✗ NaN 映射错误")
        return False
    s.reset()
    print("  ✓ NaN 映射正确 (0x7FFF)")

    # 测试 +0: 0x00000000
    pos_zero = BitVecVal(0x00000000, 32)
    fp16_pos_zero = fp32_to_fp16_symbolic(pos_zero)
    s.add(fp16_pos_zero != 0x0000)
    if s.check() == sat:
        print("  ✗ +0 映射错误")
        return False
    s.reset()
    print("  ✓ +0 映射正确 (0x0000)")

    # 测试 -0: 0x80000000
    neg_zero = BitVecVal(0x80000000, 32)
    fp16_neg_zero = fp32_to_fp16_symbolic(neg_zero)
    s.add(fp16_neg_zero != 0x8000)
    if s.check() == sat:
        print("  ✗ -0 映射错误")
        return False
    s.reset()
    print("  ✓ -0 映射正确 (0x8000)")

    print("  ✓ 所有特殊值处理正确")
    return True


def verify_roundtrip_axiomatic():
    """验证往返一致性（公理化）"""
    print("\n【公理验证：FP32 ↔ FP16 往返一致性】")

    s = Solver()

    # 符号输入
    x = BitVec('x', 32)

    # 提取组件用于假设
    sign_x, exp_x, mant_x = extract_fp32_components(x)

    # 假设 x 是正常数，且在 FP16 可精确表示范围内（113 ≤ exp ≤ 142），且尾数低13位为0（可精确表示）
    s.add(UGE(exp_x, 113))
    s.add(ULE(exp_x, 142))
    s.add((mant_x & 0x1FFF) == 0)  # 低13位为0，可无损截断

    # 执行转换
    fp16 = fp32_to_fp16_symbolic(x)
    x_restored = fp16_to_fp32_symbolic(fp16)

    # 断言：往返应等于原值
    s.add(x != x_restored)

    # 检查是否存在反例
    if s.check() == unsat:
        print("  ✓ 公理验证通过：所有可精确表示的 FP32 值往返一致")
        return True
    else:
        print("  ✗ 发现反例！")
        print(s.model())
        return False


def verify_underflow_behavior():
    """验证下溢行为"""
    print("\n【公理验证：下溢行为】")

    s = Solver()

    # 符号输入
    x = BitVec('x', 32)
    sign_x, exp_x, mant_x = extract_fp32_components(x)

    # 假设 x 的指数小于 113，应该被转换为零
    s.add(ULT(exp_x, 113))

    # 执行转换
    fp16_result = fp32_to_fp16_symbolic(x)
    
    # 检查结果是否为零（正零或负零）
    s.add(And(fp16_result != 0x0000, fp16_result != 0x8000))

    if s.check() == unsat:
        print("  ✓ 公理验证通过：所有下溢值被转换为 ±0")
        return True
    else:
        print("  ✗ 发现反例！下溢值未被正确处理")
        print(s.model())
        return False


def verify_overflow_behavior():
    """验证上溢行为"""
    print("\n【公理验证：上溢行为】")

    s = Solver()

    # 符号输入
    x = BitVec('x', 32)
    sign_x, exp_x, mant_x = extract_fp32_components(x)

    # 假设 x 的指数大于 142，应该被转换为无穷
    s.add(UGT(exp_x, 142))

    # 执行转换
    fp16_result = fp32_to_fp16_symbolic(x)
    
    # 检查结果是否为无穷（正无穷或负无穷）
    s.add(And(fp16_result != 0x7C00, fp16_result != 0xFC00))

    if s.check() == unsat:
        print("  ✓ 公理验证通过：所有上溢值被转换为 ±∞")
        return True
    else:
        print("  ✗ 发现反例！上溢值未被正确处理")
        print(s.model())
        return False


def verify_sign_preservation():
    """验证符号位保留"""
    print("\n【公理验证：符号位保留】")

    s = Solver()

    # 符号输入
    x = BitVec('x', 32)
    sign_x, _, _ = extract_fp32_components(x)

    # 执行转换
    fp16_result = fp32_to_fp16_symbolic(x)
    sign_result = LShR(fp16_result, 15) & 1

    # 符号位应保持一致
    s.add(sign_x != sign_result)

    if s.check() == unsat:
        print("  ✓ 公理验证通过：符号位正确保留")
        return True
    else:
        print("  ✗ 发现反例！符号位未被正确保留")
        print(s.model())
        return False


if __name__ == "__main__":
    print("=" * 70)
    print("FP16 理论形式化验证（使用 Z3）")
    print("=" * 70)

    all_pass = True
    all_pass &= verify_special_values()
    all_pass &= verify_sign_preservation()
    all_pass &= verify_underflow_behavior()
    all_pass &= verify_overflow_behavior()
    all_pass &= verify_roundtrip_axiomatic()

    print("\n" + "=" * 70)
    if all_pass:
        print("✓ 所有公理验证通过")
    else:
        print("✗ 公理验证失败")
    print("=" * 70)