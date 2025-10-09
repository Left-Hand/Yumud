#pragma once

#include "types/vectors/vector3.hpp"
#include "types/vectors/quat.hpp"

namespace ymd{

template<typename T>
struct Rotation3{

    [[nodiscard]] static constexpr Rotation3 from_identity(){
        return Rotation3(Matrix<T, 3, 3>::from_identity());
    }

    [[nodiscard]] static constexpr Rotation3 from_matrix(const Matrix<T, 3, 3> & matrix){
        return Rotation3(matrix);
    }

    // 绕X轴旋转
    [[nodiscard]] static constexpr Rotation3 from_rotation_x(const Angle<T> angle) {
        const auto [s, c] = angle.sincos();
        return Rotation3(Matrix<T, 3, 3>(
            T(1), T(0), T(0),
            T(0), c,    -s,
            T(0), s,    c
        ));
    }

    // 绕Y轴旋转
    [[nodiscard]] static constexpr Rotation3 from_rotation_y(const Angle<T> angle) {
        const auto [s, c] = angle.sincos();
        return Rotation3(Matrix<T, 3, 3>(
            c,    T(0), s,
            T(0), T(1), T(0),
            -s,   T(0), c
        ));
    }

    // 绕Z轴旋转
    [[nodiscard]] static constexpr Rotation3 from_rotation_z(const Angle<T> angle) {
        const auto [s, c] = angle.sincos();
        return Rotation3(Matrix<T, 3, 3>(
            c,    -s,   T(0),
            s,    c,    T(0),
            T(0), T(0), T(1)
        ));
    }

    [[nodiscard]] constexpr Vec3<T> operator * (const Vec3<T>& v) const {
        return Vec3<T>(matrix_ * to_matrix(v));
    }

    [[nodiscard]] constexpr Rotation3 inverse() const {
        return Rotation3(matrix_.transpose());
    }

    [[nodiscard]] constexpr Rotation3 transpose() const { return inverse(); }

    [[nodiscard]] constexpr Rotation3 operator * (const Rotation3& other) const { 
        return Rotation3(matrix_ * other.matrix_);
    }

    [[nodiscard]] constexpr Matrix<T, 3, 3> to_matrix() const {
        return matrix_;
    }

    [[nodiscard]] constexpr const Matrix<T, 3, 3> & matrix() const {
        return matrix_;
    }

    template<EulerAnglePolicy P>
    [[nodiscard]] constexpr EulerAngles<T, P> to_euler_angles() const {
        // https://blog.csdn.net/weixin_41010198/article/details/115960331
        const Angle<T> theta_z = Angle<T>::from_atan2(
            - matrix_.template at<2, 0>(),
            matrix_.template at<0, 0>()
        );

        const Angle<T> theta_y = Angle<T>::from_atan2(
            matrix_.template at<2, 0>(),
            imag(matrix_.template at<0, 0>(), matrix_.template at<2, 0>())
        );

        const Angle<T> theta_x = Angle<T>::from_atan2(
            matrix_.template at<2, 1>(),
            matrix_.template at<2, 2>()
        );

        return EulerAngles<T, P>::from_xyz(theta_x, theta_y, theta_z);
    }
private:
    [[nodiscard]] constexpr explicit Rotation3(const Matrix<T, 3, 3> & matrix):
        matrix_(matrix){;}

    Matrix<T, 3, 3> matrix_;
};

}