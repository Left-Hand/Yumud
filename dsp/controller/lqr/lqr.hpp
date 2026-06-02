#pragma once

#include "core/math/matrix/static_matrix.hpp"

#include "dsp/lti.hpp"

namespace ymd::dsp{

// CC 4.0 BY-SA版权
// https://blog.csdn.net/gophae/article/details/104429209

// MIT license
// https://github.com/jgsimard/RustRobotics/blob/main/src/control/lqr.rs


template<typename T, size_t S, size_t U>
static constexpr Option<math::Matrix<T, U, S>> solve_lqr(
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

template<typename T>
struct EkfResult {
    math::Matrix<T, 4, 1> x_hat; // State vector
    math::Matrix<T, 4, 4> P;     // Covariance matrix
};

/**
 * @brief Extended Kalman Filter Time Update Step
 * 
 * @tparam T Scalar type
 * @tparam S State dimension (e.g., 4 for px, py, vx, vy)
 * @tparam 2 Observation dimension (e.g., 2 for distance, angle)
 * @tparam U Control dimension (e.g., 2 for v, omega) - though here controls are passed as scalars for simplicity matching the reference
 * 
 * @param x_hat Current state estimate [px, py, vx, vy]
 * @param P Current covariance matrix
 * @param z Observation vector [distance, angle_rad]
 * @param v_body Robot linear velocity
 * @param omega_body Robot angular velocity
 * @param dt Time step
 * @param Q Process noise covariance
 * @param R Observation noise covariance
 * @param min_dist Minimum distance to avoid singularity in observation model
 * @return Option<EkfResult<T, 4, 2>> New state and covariance, or None if numerical issues occur
 */
template<typename T>
static constexpr Option<EkfResult<T>> run_ekf_t_step(
    const math::Matrix<T, 4, 1>& x_hat,
    const math::Matrix<T, 4, 4>& P,
    const math::Matrix<T, 2, 1>& z,
    T v_body,
    T omega_body,
    T dt,
    const math::Matrix<T, 4, 4>& Q,
    const math::Matrix<T, 2, 2>& R,
    T min_dist = static_cast<T>(0.2)
) {
    // Assumes 4=4, 2=2 for the specific logic below. 
    // For a fully generic version, F and H construction would need to be templated or specialized.
    // Here we implement the specific logic from the request assuming 4=4, 2=2.
    
    static_assert(4 == 4, "This implementation assumes state dimension 4");
    static_assert(2 == 2, "This implementation assumes observation dimension 2");

    // 1. Construct State Transition Matrix F_t (4x4)
    // F = [[1, -w*dt, dt, 0],
    //      [w*dt, 1, 0, dt],
    //      [0, 0, 1, 0],
    //      [0, 0, 0, 1]]
    math::Matrix<T, 4, 4> F_t;
    F_t(0, 0) = static_cast<T>(1); F_t(0, 1) = -omega_body * dt; F_t(0, 2) = dt;       F_t(0, 3) = static_cast<T>(0);
    F_t(1, 0) = omega_body * dt;   F_t(1, 1) = static_cast<T>(1); F_t(1, 2) = static_cast<T>(0); F_t(1, 3) = dt;
    F_t(2, 0) = static_cast<T>(0); F_t(2, 1) = static_cast<T>(0); F_t(2, 2) = static_cast<T>(1); F_t(2, 3) = static_cast<T>(0);
    F_t(3, 0) = static_cast<T>(0); F_t(3, 1) = static_cast<T>(0); F_t(3, 2) = static_cast<T>(0); F_t(3, 3) = static_cast<T>(1);

    // Control input u = [-v_body * dt, 0, 0, 0]^T
    math::Matrix<T, 4, 1> u_t;
    u_t(0, 0) = -v_body * dt;
    u_t(1, 0) = static_cast<T>(0);
    u_t(2, 0) = static_cast<T>(0);
    u_t(3, 0) = static_cast<T>(0);

    // 2. Prediction Step
    // x_pred = F * x_hat + u
    math::Matrix<T, 4, 1> x_pred = F_t * x_hat + u_t;
    
    // P_pred = F * P * F^T + Q
    math::Matrix<T, 4, 4> P_pred = F_t * P * F_t.transpose() + Q;

    // 3. Compute Observation Jacobian H_t and Predicted Observation h_pred
    T px = x_pred(0, 0);
    T py = x_pred(1, 0);
    T d_pred = std::sqrt(px * px + py * py);

    // Boundary check: avoid division by zero or singularity
    if (d_pred < min_dist) {
        return Some(EkfResult<T>{x_pred, P_pred});
    }

    T d_sq = d_pred * d_pred;

    // Observation Matrix H_t (2x4)
    // H = [[px/d, py/d, 0, 0],
    //      [-py/d^2, px/d^2, 0, 0]]
    math::Matrix<T, 2, 4> H_t;
    H_t(0, 0) = px / d_pred; H_t(0, 1) = py / d_pred; H_t(0, 2) = static_cast<T>(0); H_t(0, 3) = static_cast<T>(0);
    H_t(1, 0) = -py / d_sq;  H_t(1, 1) = px / d_sq;  H_t(1, 2) = static_cast<T>(0); H_t(1, 3) = static_cast<T>(0);

    // Predicted observation h(x_pred)
    math::Matrix<T, 2, 1> h_pred;
    h_pred(0, 0) = d_pred;
    h_pred(1, 0) = std::atan2(py, px);

    // 4. Update Step
    // Innovation: y = z - h_pred
    math::Matrix<T, 2, 1> y = z - h_pred;
    
    // Angle wrapping (-pi, pi) for the angle component (index 1)
    y(1, 0) = std::atan2(std::sin(y(1, 0)), std::cos(y(1, 0)));

    // Innovation Covariance: S = H * P_pred * H^T + R
    math::Matrix<T, 2, 2> S = H_t * P_pred * H_t.transpose() + R;
    
    // Kalman Gain: K = P_pred * H^T * S^-1
    auto may_S_inv = S.try_inverse();
    if (may_S_inv.is_none()) {
        return None;
    }
    math::Matrix<T, 2, 2> S_inv = may_S_inv.unwrap();
    
    math::Matrix<T, 4, 2> K = P_pred * H_t.transpose() * S_inv;
    
    // State Update: x_new = x_pred + K * y
    math::Matrix<T, 4, 1> x_hat_new = x_pred + K * y;
    
    // Covariance Update: P_new = (I - K * H) * P_pred
    math::Matrix<T, 4, 4> I_4;
    // Initialize Identity matrix
    for(size_t i=0; i<4; ++i) {
        for(size_t j=0; j<4; ++j) {
            I_4(i, j) = (i == j) ? static_cast<T>(1) : static_cast<T>(0);
        }
    }
    
    math::Matrix<T, 4, 4> P_new = (I_4 - K * H_t) * P_pred;

    return Some(EkfResult<T>{x_hat_new, P_new});
}

}