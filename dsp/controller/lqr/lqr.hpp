#pragma once

#include "core/math/matrix/static_matrix.hpp"

#include "dsp/lti.hpp"

namespace ymd::dsp{

// CC 4.0 BY-SA版权
// https://blog.csdn.net/gophae/article/details/104429209

// MIT license
// https://github.com/jgsimard/RustRobotics/blob/main/src/control/lqr.rs


template<typename T, size_t S, size_t U>
static constexpr Option<Matrix<T, U, S>> solve_lqr(
    LinearTimeInvariantModel<T, S, U> linear_model,
    size_t max_iter,
    T epsilon
){
    const auto & A = linear_model.A;
    const auto & B = linear_model.B;
    const auto & Q = linear_model.Q;
    const auto & R = linear_model.R;

    const auto A_T = A.transpose();
    const auto B_T = B.transpose();

    // Discrete time Algebraic Riccati Equation (DARE)
    auto P = linear_model.Q;

    for(size_t i = 0; i < max_iter; i++){
        const auto Pn = A_T * P * A - A_T * P * B * ({
            const auto may_mat = (R + B_T * P * B).try_inverse();
            if(may_mat.is_none()) return None;
            may_mat.unwrap();
        }) * B_T * P * A + Q;

        if((Pn - P).abs().max() < epsilon) break;
        P = Pn;
    }
    // LQR gain
    const auto k = ({
        const auto may_mat = (R + B_T * P * B).try_inverse();
        if(may_mat.is_none()) return None;
        may_mat.unwrap();
    })* B_T * P * A;

    return Some(k);
}


}