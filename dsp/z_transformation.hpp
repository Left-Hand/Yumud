#pragma once

#include "algebra/vectors/complex.hpp"
#include "algebra/vectors/polar.hpp"

namespace ymd::dsp{




template<typename T>
struct ResponseCalculator;


template<typename T, size_t N_NUM, size_t N_DEN>
struct Z_TransferCoefficients{
    std::array<T, N_NUM> num{};
    std::array<T, N_DEN> den{};

    using Self = Z_TransferCoefficients<T, N_NUM, N_DEN>;
    
    // 构造函数
    constexpr Z_TransferCoefficients() = default;
    
    constexpr Z_TransferCoefficients(const std::array<T, N_NUM>& _num, 
                                    const std::array<T, N_DEN>& _den)
        : num(_num), den(_den) {}
    
    constexpr Z_TransferCoefficients(std::array<T, N_NUM>&& _num, 
                                    std::array<T, N_DEN>&& _den)
        : num(std::move(_num)), den(std::move(_den)) {}

    // 串联操作（级联）
    template<size_t N_NUM2, size_t N_DEN2>
    [[nodiscard]] constexpr auto 
    operator*(const Z_TransferCoefficients<T, N_NUM2, N_DEN2>& other) const {
        constexpr size_t NewNUM = N_NUM + N_NUM2 - 1;
        constexpr size_t NewDEN = N_DEN + N_DEN2 - 1;
        
        std::array<T, NewNUM> new_num{};
        std::array<T, NewDEN> new_den{};
        
        // 分子卷积: num * other.num
        for (size_t i = 0; i < NewNUM; ++i) {
            for (size_t j = 0; j <= i; ++j) {
                if (j < N_NUM && (i - j) < N_NUM2) {
                    new_num[i] += num[j] * other.num[i - j];
                }
            }
        }
        
        // 分母卷积: den * other.den  
        for (size_t i = 0; i < NewDEN; ++i) {
            for (size_t j = 0; j <= i; ++j) {
                if (j < N_DEN && (i - j) < N_DEN2) {
                    new_den[i] += den[j] * other.den[i - j];
                }
            }
        }
        
        return Z_TransferCoefficients<T, NewNUM, NewDEN>(new_num, new_den);
    }
    
    // 并联操作修正版本
    template<size_t N_NUM2, size_t N_DEN2>
    [[nodiscard]] constexpr auto 
    operator+(const Z_TransferCoefficients<T, N_NUM2, N_DEN2>& other) const {
        constexpr size_t NewNUM = (N_NUM + N_DEN2 - 1 > N_NUM2 + N_DEN - 1) 
                                ? (N_NUM + N_DEN2 - 1) 
                                : (N_NUM2 + N_DEN - 1);
        constexpr size_t NewDEN = N_DEN + N_DEN2 - 1;
        
        std::array<T, NewNUM> new_num{};
        std::array<T, NewDEN> new_den{};
        
        // 公共分母: den * other.den
        for (size_t i = 0; i < NewDEN; ++i) {
            for (size_t j = 0; j <= i && j < N_DEN; ++j) {
                if (i - j < N_DEN2) {
                    new_den[i] += den[j] * other.den[i - j];
                }
            }
        }
        
        // 分子: num * other.den + other.num * den
        for (size_t i = 0; i < NewNUM; ++i) {
            // num * other.den (B1 * A2)
            for (size_t j = 0; j <= i && j < N_NUM; ++j) {
                if (i - j < N_DEN2) {
                    new_num[i] += num[j] * other.den[i - j];
                }
            }
            
            // + other.num * den (B2 * A1)
            for (size_t j = 0; j <= i && j < N_NUM2; ++j) {
                if (i - j < N_DEN) {
                    new_num[i] += other.num[j] * den[i - j];
                }
            }
        }
        
        return Z_TransferCoefficients<T, NewNUM, NewDEN>(new_num, new_den);
    }
    
    // 串联多个系统（链式调用）
    template<typename... Others>
    [[nodiscard]] constexpr auto series(Others&&... others) const {
        if constexpr (sizeof...(others) == 0) {
            return *this;
        } else {
            return (*this * ... * others);
        }
    }
    
    // 并联多个系统
    template<typename... Others>
    [[nodiscard]] constexpr auto parallel(Others&&... others) const {
        if constexpr (sizeof...(others) == 0) {
            return *this;
        } else {
            return (*this + ... + others);
        }
    }

    [[nodiscard]] constexpr auto complex_response(const auto freq, const auto fs) const {
        return ResponseCalculator<Self>::calc_complex_response(*this, freq, fs);
    }

    [[nodiscard]] constexpr Z_TransferCoefficients<T, N_NUM, N_DEN> complementary() const {
        std::array<T, N_NUM> new_num{};
        const Self& self = *this;

        // H_comp(z) = A(z) - B(z)
        new_num[0] = T(1) - self.num[0];

        if constexpr (N_NUM > 1) {
            constexpr size_t min_size = (N_NUM - 1 < N_DEN) ? (N_NUM - 1) : N_DEN;
            
            for (size_t i = 1; i <= min_size; ++i) {
                new_num[i] = self.den[i] - self.num[i];
            }
            
            // 处理剩余项（如果分子阶数高于分母）
            for (size_t i = min_size + 1; i < N_NUM; ++i) {
                new_num[i] = -self.num[i];
            }
        }
        
        return Z_TransferCoefficients<T, N_NUM, N_DEN>(new_num, self.den);
    }
};

#if 0
// 测试两个简单的一阶系统相乘
// H1(z) = (1 + z^-1) / (1 - 0.5z^-1)
// H2(z) = (1 - z^-1) / (1 + 0.5z^-1)
// H1(z) * H2(z) = [(1 + z^-1)(1 - z^-1)] / [(1 - 0.5z^-1)(1 + 0.5z^-1)]
//               = (1 - z^-2) / (1 - 0.25z^-2)

static constexpr Z_TransferCoefficients<float, 2, 2> h1{{{1.0f, 1.0f}}, {{1.0f, -0.5f}}};
static constexpr Z_TransferCoefficients<float, 2, 2> h2{{{1.0f, -1.0f}}, {{1.0f, 0.5f}}};
static constexpr auto h_series = h1 * h2;

// 验证结果
static_assert(h_series.num.size() == 3);  // 分子应该有3个系数
static_assert(h_series.den.size() == 3);  // 分母应该有3个系数
static_assert(h_series.num[0] == 1.0f);   // 分子 z^0 项系数
static_assert(h_series.num[1] == 0.0f);   // 分子 z^-1 项系数
static_assert(h_series.num[2] == -1.0f);  // 分子 z^-2 项系数
static_assert(h_series.den[0] == 1.0f);   // 分母 z^0 项系数
static_assert(h_series.den[1] == 0.0f);   // 分母 z^-1 项系数
static_assert(h_series.den[2] == -0.25f); // 分母 z^-2 项系数

// 测试两个二阶系统的并联
// H1(z) = (1 + 0.5z^-1) / (1 - 0.8z^-1 + 0.64z^-2)
// H2(z) = (1 - 0.5z^-1) / (1 + 0.4z^-1 + 0.16z^-2)
// H1(z) + H2(z) 计算较为复杂，但可以验证基本属性

static constexpr Z_TransferCoefficients<float, 2, 3> h3{{{1.0f, 0.5f}}, {{1.0f, -0.8f, 0.64f}}};
static constexpr Z_TransferCoefficients<float, 2, 3> h4{{{1.0f, -0.5f}}, {{1.0f, 0.4f, 0.16f}}};
static constexpr auto h_parallel = h3 + h4;

static_assert(h_parallel.num.size() == 4);  // 并联后分子应该有4个系数
static_assert(h_parallel.den.size() == 5);  // 并联后分母应该有5个系数

// 测试与单位系统的乘积
// H(z) * 1 = H(z)
static constexpr Z_TransferCoefficients<float, 2, 2> sys1{{{0.5f, 0.3f}}, {{1.0f, 0.2f}}};
static constexpr Z_TransferCoefficients<float, 1, 1> sys2{{{1.0f}}, {{1.0f}}};
static constexpr auto product = sys1 * sys2;

static_assert(product.num.size() == 2);
static_assert(product.den.size() == 2);
static_assert(product.num[0] == 0.5f);
static_assert(product.num[1] == 0.3f);
static_assert(product.den[0] == 1.0f);
static_assert(product.den[1] == 0.2f);

#endif

template<typename T>
struct ResponseCalculator<Z_TransferCoefficients<T, 3, 2>>{
    
    static constexpr math::Polar<T> calc_complex_response(
        const Z_TransferCoefficients<T, 3, 2>& self, 
        const auto freq, const auto fs
    ){
        const auto frequency = static_cast<T>(2.0 * M_PI) * freq / fs;

		const T cos_f = cos(frequency);
		const T sin_f = sin(frequency);
		const T cos_2f = 2 * cos_f * cos_f - 1;
		const T sin_2f = 2 * cos_f * sin_f;

        const math::Complex<T> n = {
            self.num[0] + self.num[1] * cos_f + self.num[2] * cos_2f,
            - (self.num[1] * sin_f + self.num[2] * sin_2f)
        };
		
        const math::Complex<T> d = {
            1 + self.den[0] * cos_f + self.den[1] * cos_2f,
            - (self.den[0] * sin_f + self.den[1] * sin_2f)
        };

		const auto amplitude = n.length() / d.length();
		const auto phase = (n.to_angle() - d.to_angle()).normalized();

		return {amplitude, phase};
    }
};

}