#pragma once

#include "rotation3.hpp"

namespace ymd{

    
template<typename T>
struct DhParameters{ 
    Angle<T> alpha;
    T a;
    Angle<T> theta;
    T d;

    constexpr Rotation3<T> to_rotation() const{
        const auto & self = *this;
        const auto [s_theta, c_theta] = self.theta.sincos();
        const auto [s_alpha, c_alpha] = self.alpha.sincos();

        return Rotation3<T>::from_matrix(Matrix<T, 3, 3>(
            c_theta, -s_theta * c_alpha,  s_theta * s_alpha,
            s_theta,  c_theta * c_alpha, -c_theta * s_alpha,
            T(0),     s_alpha,            c_alpha
        ));
    }

    constexpr Vec3<T> to_translation() const {
        const auto & self = *this;
        const auto [s_theta, c_theta] = self.theta.sincos();

        const auto translation = Vec3<T>(
            self.a * c_theta,
            self.a * s_theta,
            self.d
        );
    }
};



template<typename T>
struct Isometry3 { 
    using Rotation = Rotation3<T>;
    using Vec = Vec3<T>;

    static constexpr auto ZERO_1x3 = Matrix<T, 1, 3>::from_zero();
    static constexpr auto ONE_1x1 = Matrix<T, 1, 1>::from_identity();

    Rotation3<T> rotation;
    Vec3<T> translation;

    [[nodiscard]] static constexpr Isometry3 from_matrix(const Matrix<T, 4, 4>& matrix) { 
        return Isometry3{
            .rotation = Rotation3<T>::from_matrix(matrix.template submatrix<3,3>(0,0)),
            .translation = Vec3<T>(matrix.template submatrix<3,1>(0,3))
        };
    }

    [[nodiscard]] static constexpr Isometry3 from_dh_parameters(const DhParameters<T>& dh) { 
        // https://zhuanlan.zhihu.com/p/638117473

        const auto [s_theta, c_theta] = dh.theta.sincos();
        const auto [s_alpha, c_alpha] = dh.alpha.sincos();

        const auto rotation = Rotation3<T>::from_matrix(Matrix<T, 3, 3>(
            c_theta, -s_theta * c_alpha,  s_theta * s_alpha,
            s_theta,  c_theta * c_alpha, -c_theta * s_alpha,
            T(0),     s_alpha,            c_alpha
        ));

        const auto translation = Vec3<T>(
            dh.a * c_theta,
            dh.a * s_theta,
            dh.d
        );

        return Isometry3{
            .rotation = rotation,
            .translation = translation
        };
    }

    [[nodiscard]] static constexpr Isometry3<T> from_identity() {
        return Isometry3<T>{
            .rotation = Rotation3<T>::from_identity(),
            .translation = Vec3<T>::ZERO
        };
    }

    [[nodiscard]] constexpr Matrix<T, 4, 4> to_matrix() const { 
        return make_matrix_from_quad(
            rotation.to_matrix(), translation.to_matrix(),
            ZERO_1x3, ONE_1x1
        );
    }

    // 添加群运算
    [[nodiscard]] constexpr Isometry3 operator*(const Isometry3& other) const {
        return Isometry3{
            .rotation = rotation * other.rotation,
            .translation = rotation * other.translation + translation
        };
    }
    
    [[nodiscard]] constexpr Isometry3 inverse() const {
        const auto inv_rot = rotation.inverse();
        return Isometry3{
            .rotation = inv_rot,
            .translation = -(inv_rot * translation)
        };
    }
    
    // 点变换
    [[nodiscard]] constexpr Vec3<T> operator * (const Vec3<T>& point) const {
        return rotation * point + translation;
    }
    


    friend OutputStream & operator << (OutputStream & os, const Isometry3<T> & self) { 
        return os << "rotation: " << self.rotation << " translation: " << self.translation;
    }
};

}