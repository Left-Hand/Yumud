#pragma once

#include "types/vectors/vector2.hpp"


namespace ymd{


template<typename T>
struct Rotation2 {

    // 从角度构造
    [[nodiscard]] static constexpr 
    Rotation2 from_angle(Angle<T> angle) {
        const auto [s, c] = angle.sincos();
        return Rotation2{s, c};
    }


    // 恒等旋转（单位旋转）
    [[nodiscard]] static constexpr 
    Rotation2 from_identity() {
        return Rotation2{T(0), T(1)};
    }
    
    // 从旋转矩阵构造（如果矩阵是标准形式）
    [[nodiscard]] static constexpr 
    Rotation2 from_matrix(const Matrix<T, 2, 2>& mat) {
        // 假设矩阵形式为：[cos, -sin]
        //                [sin,  cos]
        return Rotation2{mat(1, 0), mat(1, 1)};
    }

    [[nodiscard]] constexpr 
    Matrix<T, 2, 2> to_matrix() const {
        return Matrix<T, 2, 2>(
            cosine_, -sine_,
            sine_,  cosine_
        );
    }

    [[nodiscard]] constexpr 
    Angle<T> angle() const {
        return Angle<T>::from_turns(atan2pu(sine_, cosine_));
    }

    [[nodiscard]] constexpr 
    Vec2<T> operator*(const Vec2<T>& v) const {
        return Vec2<T>(
            cosine_ * v.x - sine_ * v.y,
            sine_ * v.x + cosine_ * v.y
        );
    }

    [[nodiscard]] constexpr 
    Rotation2 operator*(const Rotation2& other) const {
        // 三角函数公式：sin(a+b) = sin(a)cos(b) + cos(a)sin(b)
        //             cos(a+b) = cos(a)cos(b) - sin(a)sin(b)
        return Rotation2{
            sine_ * other.cosine_ + cosine_ * other.sine_,
            cosine_ * other.cosine_ - sine_ * other.sine_
        };
    }

    // 逆旋转（转置）
    [[nodiscard]] constexpr 
    Rotation2 inverse() const {
        return Rotation2{-sine_, cosine_}; // 角度取反
    }

    [[nodiscard]] constexpr 
    Rotation2 transpose() const {
        return inverse();
    }

    [[nodiscard]] constexpr T sine() const {return sine_;}
    [[nodiscard]] constexpr T cosine() const {return cosine_;}

    #if 0
    // 插值：球面线性插值
    constexpr Rotation2 slerp(const Rotation2& other, T t) const {
        T cos_theta = cosine_ * other.cosine_ + sine_ * other.sine_;
        
        // 处理数值误差
        cos_theta = std::clamp(cos_theta, T(-1), T(1));
        
        T theta = std::acos(cos_theta);
        
        if (std::abs(theta) < 1e-6) {
            // 角度很小，直接线性插值
            return Rotation2{
                sine_ + t * (other.sine_ - sine_),
                cosine_ + t * (other.cosine_ - cosine_)
            }.normalized();
        }
        
        T sin_theta = std::sin(theta);
        T a = std::sin((1 - t) * theta) / sin_theta;
        T b = std::sin(t * theta) / sin_theta;
        
        return Rotation2{
            a * sine_ + b * other.sine_,
            a * cosine_ + b * other.cosine_
        }.normalized();
    }
    #endif


private:
    T sine_;
    T cosine_;

    constexpr explicit Rotation2(const Matrix2x2<T>& matrix):
        sine_(matrix.template at<1,0>()), cosine_(matrix.template at<1,1>()){;} 

    // 直接从sine和cosine构造
    constexpr explicit Rotation2(T sin_val, T cos_val) : sine_(sin_val), cosine_(cos_val) {;}

    friend OutputStream& operator<<(OutputStream& os, const Rotation2<T>& self) {
        return os << "Rotation2(sin=" << self.sine_ << ", cos=" << self.cosine_ 
                    << ", angle=" << self.angle().to_degrees() << "°)";
    }
};
}