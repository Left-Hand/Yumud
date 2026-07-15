"""
verify_qrt.py

对 qrt.hpp 设计草案（定点四次根 / 逆四次根，单趟牛顿迭代版本）做数值验证。
不依赖仓库里其它文件，纯 Python 原型，用整数位运算精确模拟定点寄存器行为。

分三个阶段：
  Stage A - 规范化 + 指数记账是否精确（4 bit 分块 vs sqrt.hpp 的 2 bit 分块）
  Stage B - 逆四次根牛顿迭代 g' = g*(1.25 - 0.25*x*g^4) 的收敛速度（高精度实数）
  Stage C - 用 32 bit 截断乘法（模拟 mul32hu）重跑一遍，看定点精度是否够用
"""

import random

MASK32 = (1 << 32) - 1
random.seed(12345)


def clz32(x):
    assert 0 <= x <= MASK32
    if x == 0:
        return 32
    return 32 - x.bit_length()


# =====================================================================
# Stage A: 规范化 / 指数记账正确性
# =====================================================================

def normalize_quart(x_bits, Q, inverse):
    """
    x_bits: 32-bit 无符号寄存器, 真实值 V = x_bits / 2**Q
    inverse=False -> 为计算 x^(1/4)  做准备
    inverse=True  -> 为计算 x^(-1/4) 做准备

    返回 (mantissa, exponent)：
        mantissa 落在 [0x10000000, 0xFFFFFFFF]（顶 bit 在 28~31 之间）
        exponent 满足：mantissa_real ** power * 2**exponent == x_real ** power
        （power = 0.25 或 -0.25，取决于 inverse）
    """
    if x_bits == 0:
        return 0, 0

    base = 32 - Q
    r = base % 4
    if r:
        assert (x_bits >> (32 - r)) == 0, (
            f"Q={Q}: 输入没有 {r} bit 的余量做预左移 (x_bits=0x{x_bits:08x})"
        )
        x_bits <<= r
    exponent = (base - r) // 4
    if inverse:
        exponent = -exponent

    lz = clz32(x_bits)
    chunks = lz // 4
    if chunks:
        x_bits <<= 4 * chunks
        if inverse:
            exponent += chunks
        else:
            exponent -= chunks

    assert 0x10000000 <= x_bits <= MASK32, hex(x_bits)
    return x_bits, exponent


def stage_a():
    print("=== Stage A: 规范化 + 指数记账 ===")
    worst = 0.0
    trials = 0
    for Q in range(0, 32):
        headroom = (32 - Q) % 4  # 需要给预左移留出的高位余量
        hi = (1 << (32 - headroom)) - 1
        if hi <= 0:
            continue
        for _ in range(500):
            x_bits = random.randint(1, hi)
            V = x_bits / (2 ** Q)

            for inverse in (False, True):
                m, e = normalize_quart(x_bits, Q, inverse)
                m_real = m / (2 ** 32)
                power = -0.25 if inverse else 0.25
                lhs = (m_real ** power) * (2.0 ** e)
                rhs = V ** power
                rel_err = abs(lhs - rhs) / rhs
                worst = max(worst, rel_err)
                trials += 1
                assert rel_err < 1e-9, (Q, inverse, x_bits, lhs, rhs, rel_err)
    print(f"  {trials} 次试验通过，最大相对误差 = {worst:.3e}")


# =====================================================================
# Stage B: 逆四次根牛顿迭代收敛速度（高精度实数模拟）
# =====================================================================

TABLE_BITS = 7  # 先仿照 sqrt.hpp 用 7 bit 索引，之后再决定是否要扩宽


def build_naive_table(bits):
    """
    m 的规范化范围是 [1/16, 1)。用 `bits` 位对该区间做等分索引，
    每个桶取区间中点的真实 m^-0.25 作为初值（相当于最朴素的查表，
    不是精细拟合，用来测"最坏情况下需要几次迭代"）。
    """
    n = 1 << bits
    table = []
    for i in range(n):
        # 区间 [1/16, 1) 按 i 等分
        m_lo = 1 / 16 + i * (1 - 1 / 16) / n
        m_mid = m_lo + (1 - 1 / 16) / n / 2
        table.append(m_mid ** -0.25)
    return table


def lookup_guess(m, table, bits):
    n = 1 << bits
    idx = int((m - 1 / 16) / (1 - 1 / 16) * n)
    idx = min(max(idx, 0), n - 1)
    return table[idx]


def newton_inv4th_real(m, g0, iters, trace=False):
    g = g0
    for i in range(iters):
        g = g * (1.25 - 0.25 * m * g ** 4)
        if trace:
            err = abs(g - m ** -0.25) / (m ** -0.25)
            print(f"    iter {i}: g={g:.12f} rel_err={err:.3e}")
    return g


def stage_b():
    print("=== Stage B: 逆四次根牛顿迭代收敛（实数精度） ===")
    table = build_naive_table(TABLE_BITS)

    target_rel_err = 2 ** -30  # 对齐 IQ32 定点精度需求

    for iters in (1, 2, 3, 4):
        worst = 0.0
        for _ in range(5000):
            m = random.uniform(1 / 16, 1.0)
            g0 = lookup_guess(m, table, TABLE_BITS)
            g = newton_inv4th_real(m, g0, iters)
            true = m ** -0.25
            rel_err = abs(g - true) / true
            worst = max(worst, rel_err)
        ok = "OK" if worst < target_rel_err else "不够"
        print(f"  迭代 {iters} 次, {TABLE_BITS}-bit 查表: 最大相对误差 = {worst:.3e}  [{ok}]")

    # 正四次根恒等式校验: x^(1/4) = x * g^3, g = x^(-1/4)
    print("  正四次根恒等式 x^(1/4) = x * g^3 校验:")
    worst = 0.0
    for _ in range(5000):
        m = random.uniform(1 / 16, 1.0)
        g0 = lookup_guess(m, table, TABLE_BITS)
        g = newton_inv4th_real(m, g0, 3)
        qrt_via_identity = m * g ** 3
        qrt_true = m ** 0.25
        rel_err = abs(qrt_via_identity - qrt_true) / qrt_true
        worst = max(worst, rel_err)
    print(f"    最大相对误差 = {worst:.3e}")


# =====================================================================
# Stage C: 32-bit 截断乘法（模拟 mul32hu）定点精度是否够用
# =====================================================================

def mul32hu(a, b):
    """a, b: UQ0.32 (0 <= . < 2**32)，返回高 32 位，即 round-down(a*b/2**32)。"""
    return (a * b) >> 32


def to_fixed(x, frac_bits):
    return int(round(x * (1 << frac_bits))) & MASK32


def from_fixed(x, frac_bits):
    return x / (1 << frac_bits)


def newton_inv4th_fixed32(m_bits, g0_bits, iters, trace=False):
    """
    全程用 32-bit 截断乘法(mul32hu)模拟硬件实现。
    m_bits: UQ0.32, m in [1/16, 1)
    g0_bits: UQ2.30, g in (1, 2]   (2 个整数位 + 30 个小数位)
    返回: UQ2.30 g_bits
    """
    FRAC = 30  # g 的小数位数 (2 个整数位)
    g = g0_bits  # UQ2.30

    for i in range(iters):
        # g^2: (UQ2.30 * UQ2.30) 高32位 -> UQ4.28，再对齐回 UQ? 需要缩放
        # 用 mul32hu 处理时按 UQ0.32 输入/输出的方式统一换算：
        # 把 g (UQ2.30, 值域(1,2]) 左移 2 bit 看成 UQ0.32 的 (值域(0.25,0.5])
        g_as_uq32 = (g << 2) & MASK32          # UQ0.32, 表示 g/4
        g2_as_uq32 = mul32hu(g_as_uq32, g_as_uq32)  # (g/4)^2 的 UQ0.32
        # g2_as_uq32 表示 (g/4)^2 = g^2/16, 值域 (1/16, 1/4]
        g4_as_uq32 = mul32hu(g2_as_uq32, g2_as_uq32) << 4 & MASK32
        # g4_as_uq32 表示 (g^2/16)^2 * 16 = g^4/16, 值域约 (1/16,1]... 需要仔细验证缩放
        # (缩放系数在下面用真实值对照校验，而不是手推)
        mg4 = mul32hu(m_bits, g4_as_uq32)      # m * g^4 / 16, UQ0.32
        corr_const = to_fixed(1.25 / 4, 32)     # 1.25/4 对齐 mg4 的缩放
        corr = corr_const - (mg4 >> 2)
        g_as_uq32_new = mul32hu(g_as_uq32, corr) << 2 & MASK32
        g = (g_as_uq32_new >> 2) & MASK32
        if trace:
            print(f"    iter {i}: g={from_fixed(g, FRAC):.10f}")
    return g


def stage_c():
    print("=== Stage C: 32-bit 截断乘法定点精度 ===")
    print("  (先用简化/未必最优的缩放方案跑一遍，量化误差数量级，")
    print("   不追求本阶段就是最终实现)")
    worst = 0.0
    n = 0
    table = build_naive_table(TABLE_BITS)
    for _ in range(2000):
        m = random.uniform(1 / 16, 1.0)
        m_bits = to_fixed(m, 32)
        g0 = lookup_guess(m, table, TABLE_BITS)
        g0_bits = to_fixed(g0, 30)
        g_bits = newton_inv4th_fixed32(m_bits, g0_bits, 3)
        g_val = from_fixed(g_bits, 30)
        true = m ** -0.25
        if g_val <= 0:
            continue
        rel_err = abs(g_val - true) / true
        worst = max(worst, rel_err)
        n += 1
    print(f"  {n} 次试验，32-bit 截断乘法版最大相对误差 = {worst:.3e}")
    print(f"  (目标 ~2^-30 = {2**-30:.3e}；若明显不够，说明 g^2->g^4 这条乘法链")
    print(f"   需要比 sqrt 版本更高的中间精度，例如用 64x64->64 的 mul64hu)")


if __name__ == "__main__":
    stage_a()
    print()
    stage_b()
    print()
    stage_c()
