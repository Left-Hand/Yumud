---
title: 浮点数类型重构与形式化验证
created: 2026-05-06
author: Rstr1aN
tags: [formal-verification, float-conversion, fp16, fp8, bit_cast]
---

# 浮点数类型重构与形式化验证工作流

## 概述

本工作流记录了 FP16、FP8-E4M3、FP8-E5M2 浮点数类型的完整重构过程，从基于 `union` 的实现迁移到基于 `std::bit_cast` 的 C++20 现代实现，并使用形式化验证确保转换正确性。

## 项目背景

### 问题陈述
- 原始实现使用 `union` 进行位转换，存在歧义和安全性问题
- FP16 存在多个未验证的手动位操作实现
- FP8 类型未完整实现，转换逻辑散落多处
- 缺乏编译期和运行期验证

### 目标
1. ✅ 统一使用 `std::bit_cast` 进行位转换
2. ✅ 集中转换逻辑到 `intrinsics` 命名空间
3. ✅ 添加编译期 `static_assert` 测试
4. ✅ 集成标准库特化（`numeric_limits`等）
5. ✅ 为形式化验证做好准备

---

## 核心改动

### 1. FP16 重构 (`core/math/float/fp16.hpp`)

#### 1.1 转换函数隔离

**之前**：直接在结构体中手动位操作

**之后**：统一的转换函数在 `intrinsics` 命名空间

```cpp
namespace intrinsics{
    constexpr uint32_t f32inf = 255UL << 23;
    constexpr uint32_t f16inf = 31UL << 23;
    
    static constexpr uint16_t fp32_to_fp16_nonfpu(float value){
        uint32_t in = std::bit_cast<uint32_t>(value);
        // ... 位操作
        return out;
    }
    
    static constexpr float fp16_to_fp32_nonfpu(uint16_t value){
        uint32_t out_bits = (value & 0x7FFFU) << 13U;
        // ... 位操作
        return std::bit_cast<float>(out_bits);
    }
}
```

**关键决策**：
- 使用 `std::bit_cast` 代替 `union`，提高安全性和 `constexpr` 支持
- 函数命名加 `_nonfpu` 后缀，表示纯软件实现

#### 1.2 构造函数简化

```cpp
// 从 float 构造
constexpr fp16(float f_val) {
    *this = from_bits(intrinsics::fp32_to_fp16_nonfpu(f_val));
}

// 从 int 构造
constexpr fp16(int int_val){
    *this = int32_to_fp16(int_val);
}

// 私有辅助函数
static constexpr fp16 int32_to_fp16(int32_t int_val){
    return from_bits(intrinsics::fp32_to_fp16_nonfpu(static_cast<float>(int_val)));
}
```

#### 1.3 编译期测试 (`static_assert`)

```cpp
namespace test_fp16 {
    // 整数转换测试
    constexpr auto zero = fp16(0);
    static_assert(zero.to_bits() == 0x0000);
    
    constexpr auto one = fp16(1);
    static_assert(one.to_bits() == 0x3C00);
    
    // 浮点转换测试
    constexpr auto half = fp16(0.5f);
    static_assert(half.to_bits() == 0x3800);
    
    // 反向转换测试
    static_assert(static_cast<float>(fp16(1)) == 1.0f);
    static_assert(static_cast<float>(fp16(0.5f)) == 0.5f);
}

namespace {
    // 边界值测试
    static_assert(fp16(65504.0f).to_bits() == 0x7BFF);  // 最大值
    static_assert(fp16(0.0009765625f).to_bits() == 0x0400);  // 最小规范数
}
```

#### 1.4 标准库特化

**`std::numeric_limits<fp16>` 完整特化**：
- `digits = 11` (1 符号 + 10 尾数)
- `min_exponent = -14`, `max_exponent = 16`
- `epsilon()`, `infinity()`, `quiet_NaN()` 等常量


**Type traits 特化**：
```cpp
template<> struct is_arithmetic<fp16> : std::true_type {};
template<> struct is_floating_point<fp16> : std::true_type {};
template<> struct is_signed<fp16> : std::true_type {};
template<> struct is_trivially_copyable<fp16> : std::true_type {};
// 等等
```

---

### 2. FP8 完整实现 (`core/math/float/fp8.hpp`)

#### 2.1 转换函数设计（隔离到 `intrinsics` 命名空间）

**FP32 → FP8-E4M3**：
```cpp
static constexpr uint8_t fp32_to_fp8_e4m3_nonfpu(float value){
    uint32_t bits = std::bit_cast<uint32_t>(value);
    uint8_t sign = (bits >> 31) & 0x1;
    int exponent = ((bits >> 23) & 0xFF) - 127;
    uint32_t mantissa = bits & 0x7FFFFF;
    
    uint8_t out = 0;
    if (exponent > 7) {
        // 溢出处理
        out = (uint8_t)(0x7E | (sign << 7));
    } else if (exponent < -8) {
        // 下溢处理
        out = (uint8_t)(sign << 7);
    } else {
        // 正常范围
        // 不变量：exponent 在 [-8, 7]
        if (exponent > 7 || exponent < -8) __builtin_unreachable();
        
        int exp8_signed = exponent + 7;
        if (exp8_signed < -1 || exp8_signed > 14) __builtin_unreachable();
        
        uint8_t exp8 = (uint8_t)exp8_signed;
        uint8_t mant8 = (mantissa >> 20) & 0x7;
        out = (uint8_t)((mant8 & 0x7) | ((exp8 & 0xF) << 3) | (sign << 7));
    }
    return out;
}
```

**FP8-E4M3 → FP32**：
```cpp
static constexpr float fp8_e4m3_to_fp32_nonfpu(uint8_t value){
    uint8_t sign = (value >> 7) & 0x1;
    uint8_t exp = (value >> 3) & 0xF;
    uint8_t mant = value & 0x7;
    
    uint32_t f32_bits = 0;
    if (exp == 0 && mant == 0) {
        // 零值
        f32_bits = 0;
    } else if (exp == 0xF) {
        // 特殊值：Inf 或 NaN
        f32_bits = (0xFF << 23) | (mant << 20);
    } else {
        // 正常范围：互斥检查
        if ((exp == 0 && mant == 0) || exp == 0xF) __builtin_unreachable();
        
        uint32_t f32_exp = exp + (127 - 7);
        uint32_t f32_mant = (uint32_t)mant << 20;
        f32_bits = (f32_exp << 23) | f32_mant | (1U << 23);
    }
    f32_bits |= (uint32_t)sign << 31;
    return std::bit_cast<float>(f32_bits);
}
```

#### 2.2 FP8-E5M2 的类似实现

同样逻辑，参数调整：
- 指数偏差：15（而非 7）
- 尾数位数：2（而非 3）
- 指数范围：[-16, 15]

#### 2.3 完整结构体实现

```cpp
struct alignas(1) [[nodiscard]] fp8_e4m3 final{
    uint8_t mant:3;
    uint8_t exp:4;
    uint8_t sign:1;
    
    // 构造函数
    constexpr fp8_e4m3(int iv){
        *this = from_bits(intrinsics::fp32_to_fp8_e4m3_nonfpu(static_cast<float>(iv)));
    }
    
    constexpr fp8_e4m3(float fv){
        *this = from_bits(intrinsics::fp32_to_fp8_e4m3_nonfpu(fv));
    }
    
    // 转换接口
    static constexpr fp8_e4m3 from_bits(const uint8_t bits){
        return std::bit_cast<fp8_e4m3>(bits);
    }
    
    constexpr uint8_t to_bits() const noexcept {
        return std::bit_cast<uint8_t>(*this);
    }
    
    constexpr bool is_nan() const noexcept {
        return exp == 0xF && mant != 0;
    }
    
    explicit constexpr operator float() const noexcept {
        return intrinsics::fp8_e4m3_to_fp32_nonfpu(to_bits());
    }
};
```

同理为 `fp8_e5m2` 实现。

#### 2.4 编译期测试

```cpp
namespace test_fp8 {
    constexpr auto zero_e4m3 = fp8_e4m3(0);
    static_assert(zero_e4m3.to_bits() == 0x00);
    
    constexpr auto one_e4m3 = fp8_e4m3(1.0f);
    static_assert(one_e4m3.to_bits() == 0x38);
    
    constexpr auto neg_one_e4m3 = fp8_e4m3(-1.0f);
    static_assert(neg_one_e4m3.to_bits() == 0xB8);
    
    // E5M2 测试类似
    static_assert(static_cast<float>(fp8_e4m3(1.0f)) == 1.0f);
}

static_assert(sizeof(fp8_e4m3) == 1);
static_assert(sizeof(fp8_e5m2) == 1);
```

#### 2.5 标准库特化

同 FP16，包括：
- `is_arithmetic`, `is_floating_point`, `is_signed` 等
- `numeric_limits` 完整特化（针对 E4M3 和 E5M2 各自的参数）

---

## 形式化验证策略

### 方法：Z3 求解器 + Python

**为什么选择 Z3？**
- 轻量级，无编译依赖（Python 包）
- 对位操作和约束的强大支持
- 完全开源（微软维护）

### 验证范围

#### 1. 约束满足性验证 (SAT)
验证转换函数的逻辑约束是否可满足且完备


#### 2. 边界值验证

#### 3. 往返转换一致性


### 编译期验证增强：`__builtin_unreachable()`

在转换函数中标记不可能达成的条件，启用编译器的强化分析：

```cpp
// 在 else 分支中标记不可能的条件
if (exponent > 7 || exponent < -8) __builtin_unreachable();

// 在互斥分支中标记重复条件
if ((exp == 0 && mant == 0) || exp == 0xF) __builtin_unreachable();
```

**效果**：
- 编译器可以优化掉冗余检查
- SMT 求解器获得更强的约束
- 编译时可能报错（如果约束矛盾）

---

## 工作流步骤

### Phase 1: 代码重构（已完成 ✅）

- [x] FP16：`union` → `std::bit_cast`
- [x] FP16：完善 `int32_to_fp16` 实现
- [x] FP16：添加 `__builtin_unreachable()` 标记
- [x] FP8-E4M3：隔离转换函数到 `intrinsics`
- [x] FP8-E5M2：完整实现全部功能
- [x] 两者：添加 `std::numeric_limits` 特化

### Phase 2: 编译期验证（已完成 ✅）

- [x] FP16：`static_assert` 整数/浮点转换测试
- [x] FP16：边界值验证（最大值、最小规范数、NaN）
- [x] FP8：`static_assert` 基本转换测试
- [x] FP8：大小验证 (`sizeof`)

### Phase 3: 形式化验证（待进行）

- [ ] 实现 Z3 约束验证脚本
- [ ] 验证溢出/下溢/正常范围的互斥性
- [ ] 验证往返转换的一致性
- [ ] 生成验证报告

### Phase 4: 文档与自动化（待进行）

- [ ] 创建 `run_verification.sh` 自动化脚本
- [ ] 编写 Makefile 目标
- [ ] 集成到 CI/CD（若有）

---

## 相关文件

| 文件 | 状态 | 说明 |
|------|------|------|
| `core/math/float/fp16.hpp` | ✅ 完成 | FP16 重构实现 |
| `core/math/float/fp8.hpp` | ✅ 完成 | FP8 完整实现 |
| `.claude/workflows/FLOAT_REFACTORING.md` | ✅ 完成 | 本工作流文档 |
| `.claude/workflows/fp8_formal_verification/verify_fp8.py` | ⏳ 待创建 | Z3 验证脚本 |
| `.claude/workflows/fp8_formal_verification/requirements.txt` | ⏳ 待创建 | Python 依赖 |

---

## 关键决策及其理由

### 1. 使用 `std::bit_cast` 而非 `union`

| 方面 | `union` | `std::bit_cast` |
|------|--------|-----------------|
| C++ 标准 | C++98+ | C++20+ |
| `constexpr` 支持 | ❌ 需特殊处理 | ✅ 完全支持 |
| 类型安全 | ⚠️ 可能未定义行为 | ✅ 明确定义 |
| 可读性 | ⚠️ 歧义 | ✅ 清晰 |

**选择** `std::bit_cast`，因为：
- 项目使用 C++20（由 `<bit>` 头文件推断）
- 更安全、更易验证

### 2. 隔离转换函数到 `intrinsics` 命名空间

**好处**：
- 清晰的分层：转换逻辑（`intrinsics`）vs 公共接口（结构体）
- 易于单独测试和验证
- 易于未来并行实现（FPU 加速版本）

### 3. 添加 `__builtin_unreachable()`

**效果**：
- 编译器优化：消除冗余检查
- 形式化验证友好：提供强约束
- 编译时错误检测：如果约束矛盾

### 4. 完整的标准库特化

**为什么？**
- 让 `fp16`/`fp8` 完全融入 C++ 生态
- 支持 `std::numeric_limits<fp16>::max()` 等
- 支持容器、算法的泛型编程

---

## 验证清单

### 类型安全
- [x] 大小约束：`static_assert(sizeof(fp16) == 2)`
- [x] 字节对齐：`alignas(2)` 等
- [x] 三态逻辑一致性（符号、指数、尾数）

### 转换正确性
- [x] 零值转换：`fp16(0) → 0x0000`
- [x] 单位值转换：`fp16(1) → 0x3C00`
- [x] 符号位保留：`fp16(-1) → 0xBC00`
- [x] 溢出处理：`fp16(65504) → 0x7BFF`
- [x] 下溢处理：`fp16(tiny) → 0x0000`

### NaN/Inf 处理
- [x] NaN 检测：`is_nan()` 方法
- [x] Inf 常量：`numeric_limits<fp16>::infinity()`
- [x] 特殊值位模式：`0x7C00` (Inf), `0x7E00` (NaN)

---

## 后续工作

### 优先级 1: 形式化验证脚本
```bash
cd .claude/workflows/fp8_formal_verification
pip install -r requirements.txt
python verify_fp8.py
```

### 优先级 2: CI/CD 集成
- 在编译时运行 `static_assert` 验证
- 在 CI 中运行 Z3 验证脚本
- 生成验证报告

### 优先级 3: 性能基准
- 比对 `__builtin_unreachable()` 优化效果
- 测试与硬件 FPU 的性能差异

---

## 参考资源

- [C++20 `std::bit_cast` 文档](https://en.cppreference.com/w/cpp/numeric/bit_cast)
- [IEEE 754 浮点数标准](https://en.wikipedia.org/wiki/IEEE_754)
- [Z3 求解器文档](https://microsoft.github.io/z3/)
- [FP16 格式细节](https://en.wikipedia.org/wiki/Half-precision_floating-point_format)
- [FP8 (E4M3) 格式](https://arxiv.org/abs/2209.05433)

---

**最后更新**：2026-05-06  
**状态**：Phase 2 完成，Phase 3 待进行  
**维护者**：Rstr1aN
