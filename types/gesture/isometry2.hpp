#pragma once

#include "rotation2.hpp"
#include "types/vectors/complex.hpp"

namespace ymd{

template<typename T>
struct IsometryMatrix2 { 
};

template<typename T>
struct Isometry2 { 
    using Rotation = UnitComplex<T>;
    using Vec = Vec2<T>;


    UnitComplex<T> rotation;
    Vec2<T> translation;

    [[nodiscard]] static constexpr Isometry2 from_matrix(const Matrix<T, 3, 3>& matrix) { 
        return Isometry2{
            .rotation = UnitComplex<T>(matrix.template at<0,0>(), matrix.template at<0,1>()),
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
    static constexpr auto ZERO_1x2 = Matrix<T, 1, 2>::zero();
    static constexpr auto ONE_1x1 = Matrix<T, 1, 1>::from_identity();

};
}