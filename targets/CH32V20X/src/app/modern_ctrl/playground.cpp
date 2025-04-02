#include "LQR.hpp"

// https://blog.csdn.net/gophae/article/details/104429209

using namespace ymd;
using namespace ymd::modctrl;

static constexpr double EPS = 1e-6;
static constexpr size_t MAX_ITER = 150;
// def solve_DARE(A, B, Q, R, maxiter=150, EPS=0.01):
//     """
//     Solve a discrete time_Algebraic Riccati equation (DARE)
//     """
//     P = Q

//     for i in range(maxiter):
//         Pn = A.T @ P @ A - A.T @ P @ B @
//             inv(R + B.T @ P @ B) @ B.T @ P @ A + Q
//         if (abs(Pn - P)).max() < EPS:
//             break
//         P = Pn

//     return Pn

// Function to solve the Discrete Algebraic Riccati Equation (DARE)
template <typename T, size_t Xn, size_t Un>
constexpr Matrix_t<T, Xn, Xn> solve_DARE(
    const Matrix_t<T, Xn, Xn>& A,  // State transition matrix
    const Matrix_t<T, Xn, Un>& B,  // Control matrix
    const Matrix_t<T, Xn, Xn>& Q,  // State cost matrix
    const Matrix_t<T, Un, Un>& R,  // Control cost matrix
    size_t max_iter = MAX_ITER,         // Maximum number of iterations
    T tolerance = T(EPS)) {          // Convergence tolerance

    Matrix_t<T, Xn, Xn> P = Q;     // Initialize the cost matrix

    for (size_t iter = 0; iter < max_iter; ++iter) {
        // Compute the next iteration of P
        // Matrix_t<T, Xn, Xn> Pn = A.transpose() * P * A -
        //                          A.transpose() * P * B *
        //                          (R + (B.transpose() * P * B)).inverse() *
        //                          B.transpose() * P * A + Q;

        const auto Pn = (A.transpose() * P * A) -
                        ((A.transpose() * P * B) *
                        ((R + (B.transpose() * P * B)).inverse() *
                        (B.transpose() * P * A))) + Q;

        // Check for convergence
        T diff = 0;
        for (size_t i = 0; i < Xn; ++i) {
            for (size_t j = 0; j < Xn; ++j) {
                diff = std::max(diff, std::abs(Pn(i, j) - P(i, j)));
            }
        }

        if (diff < tolerance) {
            break;
        }

        P = Pn;
    }

    return P;
}

// LQR求解函数
template <typename T, size_t Xn, size_t Un>
constexpr 
Matrix_t<T, Un, Xn> solveLQR(
    const Matrix_t<T, Xn, Xn>& A,  // 状态转移矩阵
    const Matrix_t<T, Xn, Un>& B,  // 控制矩阵
    const Matrix_t<T, Xn, Xn>& Q,  // 状态代价矩阵
    const Matrix_t<T, Un, Un>& R,  // 控制代价矩阵
    size_t max_iter = MAX_ITER,       // 最大迭代次数
    T tolerance = T(EPS)) {        // 收敛容差

    const auto P = solve_DARE(A, B, Q, R, max_iter, tolerance);
    return ((B.transpose() * P * B) + R).inverse() * (B.transpose() * P * A);
    // return R.inverse() * B.transpose() * P;
}

void test_lqr(){

    // Define system matrices for a simple LQR problem
    constexpr size_t Xn = 2; // State dimension
    constexpr size_t Un = 1; // Control input dimension

    // State transition matrix A
    constexpr Matrix_t<double, Xn, Xn> A{
        0.0, 1.0,
        10.0, 0.0
    };

    // Control matrix B
    constexpr Matrix_t<double, Xn, Un> B{
        0.0,
        -1.0
    };

    // State cost matrix Q
    constexpr Matrix_t<double, Xn, Xn> Q{
        1.0, 0.0,
        0.0, 0.1
    };

    [[maybe_unused]]constexpr auto Q_ = Q.inverse();
    // constexpr auto I = Q * Q_;
    // [[maybe_unused]]constexpr auto Q_2 = Q.guassian_inverse();

    // Control cost matrix R
    constexpr Matrix_t<double, Un, Un> R{
        1.0
    };

    // Solve LQR
    [[maybe_unused]] constexpr auto P = solve_DARE(A, B, Q, R);
    [[maybe_unused]] constexpr auto K = solveLQR(A, B, Q, R);

}