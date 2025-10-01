#pragma once

#include "rotation2.hpp"

namespace ymd{


template<typename T>
struct Isometry2 { 
    using Rotation = Rotation2<T>;
    using Vec = Vec2<T>;


    Rotation2<T> rotation;
    Vec2<T> translation;

    [[nodiscard]] static constexpr Isometry2 from_matrix(const Matrix<T, 3, 3>& matrix) { 
        return Isometry2{
            .rotation = Rotation2<T>::from_matrix(matrix.template submatrix<2,2>(0,0)),
            .translation = Vec2<T>(matrix.template submatrix<2,1>(0,2))
        };
    }

    [[nodiscard]] constexpr Matrix<T, 3, 3> to_matrix() const { 
        return make_matrix_from_quad(
            rotation.to_matrix(), translation.to_matrix(),
            ZERO_1x2, ONE_1x1
        );
    }

    friend OutputStream & operator << (OutputStream & os, const Isometry2<T> & self) { 
        return os << "rotation: " << self.rotation << " translation: " << self.translation;
    }

private:
    static constexpr auto ZERO_1x2 = Matrix<T, 1, 2>::from_zero();
    static constexpr auto ONE_1x1 = Matrix<T, 1, 1>::from_identity();

};
}