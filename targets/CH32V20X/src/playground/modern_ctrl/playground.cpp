#include "dsp/controller/lqr/lqr.hpp"

using namespace ymd;

void test_lqr(){

    // Define system matrices for a simple LQR problem
    constexpr size_t Xn = 2; // State dimension
    constexpr size_t Un = 1; // Control input dimension

    // State transition matrix A
    constexpr auto A = math::Matrix<float, Xn, Xn>{
        0.0, 1.0,
        10.0, 0.0
    };

    // Control matrix B
    constexpr auto B = math::Matrix<float, Xn, Un>{
        0.0,
        -1.0
    };

    // State cost matrix Q
    constexpr auto Q = math::Matrix<float, Xn, Xn>{
        1.0, 0.0,
        0.0, 0.1
    };

    // Control cost matrix R
    constexpr math::Matrix<float, Un, Un> R{
        1.0
    };

    constexpr auto model = dsp::make_lti_model(
        A, B, R, Q
    );

    
    [[maybe_unused]]constexpr auto Q_ = Q.inverse();
    // constexpr auto I = Q * Q_;
    // [[maybe_unused]]constexpr auto Q_2 = Q.guassian_inverse();
    // Solve LQR
    // [[maybe_unused]] constexpr auto P = solve_DARE(A, B, Q, R);
    [[maybe_unused]] constexpr auto K = dsp::solve_lqr<float>(model, 150, 0.001).unwrap();

}