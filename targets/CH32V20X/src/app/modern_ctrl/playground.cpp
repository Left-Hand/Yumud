#include "LQR.hpp"

using namespace ymd;
using namespace ymd::modctrl;

void test_lqr(){

    // Define system matrices for a simple LQR problem
    constexpr size_t Xn = 2; // State dimension
    constexpr size_t Un = 1; // Control input dimension

    // State transition matrix A
    constexpr Matrix<double, Xn, Xn> A{
        0.0, 1.0,
        10.0, 0.0
    };

    // Control matrix B
    constexpr Matrix<double, Xn, Un> B{
        0.0,
        -1.0
    };

    // State cost matrix Q
    constexpr Matrix<double, Xn, Xn> Q{
        1.0, 0.0,
        0.0, 0.1
    };



    // Control cost matrix R
    constexpr Matrix<double, Un, Un> R{
        1.0
    };

    
    [[maybe_unused]]constexpr auto Q_ = Q.inverse();
    // constexpr auto I = Q * Q_;
    // [[maybe_unused]]constexpr auto Q_2 = Q.guassian_inverse();
    // Solve LQR
    [[maybe_unused]] constexpr auto P = solve_DARE(A, B, Q, R);
    [[maybe_unused]] constexpr auto K = solveLQR(A, B, Q, R);

}