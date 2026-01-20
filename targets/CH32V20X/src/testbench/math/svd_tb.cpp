#include <bitset>
#include <ranges>


#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/Result.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "hal/bus/uart/uarthw.hpp"

#include "robots/slam/svd.hpp"
#include "algebra/vectors/vec3.hpp"
#include "algebra/vectors/quat.hpp"

#include "algebra/gesture/isometry2.hpp"
#include "algebra/gesture/isometry3.hpp"

#include "core/string/view/string_view.hpp"

using namespace ymd;

#define UART hal::uart6
static constexpr size_t UART_BAUD = 576000u;


enum class SlamErrorKind:uint8_t{
    PointsCntNotMatch,
    InsufficientPoints,
    JacobiSvdIterLimitReached,
};

OutputStream & operator<<(OutputStream & os, const SlamErrorKind & error){
    switch(error){
        case SlamErrorKind::PointsCntNotMatch: return os << "PointsCntNotMatch";
        case SlamErrorKind::InsufficientPoints: return os << "InsufficientPoints";
        case SlamErrorKind::JacobiSvdIterLimitReached: return os << "JacobiSvdIterLimitReached";
        default: __builtin_unreachable(); 
    }
}

// MIT license
// https://github.com/jgsimard/RustRobotics/blob/main/src/mapping/se2_se3.rs

namespace ymd::slam{

template<typename T>
static constexpr Matrix<T, 3, 9> jacobian_so3(const Matrix3x3<T> & m){
    const auto trace = m.trace();
    const auto c = sqrt(trace - 1) * static_cast<T>(0.5);
    if(c > static_cast<T>(0.999999))
        return Matrix<T, 3, 9>::zero();
    const auto s = sqrt(1 - math::square(c));

    const auto theta = atan2(s, c);
    const auto factor = (theta * c - s) / (4  * s * s * s);
    const auto a1 = m.template at<2,1> - m.template at<1,2> * factor;
    const auto a2 = m.template at<0,2> - m.template at<2,0> * factor;
    const auto a3 = m.template at<1,0> - m.template at<0,1> * factor;
    const auto b = static_cast<T>(0.5) * theta / s;

    return Matrix<T, 3, 9>(
        a1,  a2,  a3,  
        0.0, 0.0,   b,
        0.0,  -b, 0.0,
        0.0, 0.0,  -b,
        a1,  a2,  a3,
        b, 0.0, 0.0,
        0.0,   b, 0.0,
        -b, 0.0, 0.0,
        a1,  a2,  a3
    );
}
}



namespace ymd::slam::details{


namespace experimental{ 


    template<typename T>    
    __fast_inline Rotation2<T> svd_2x2(const Matrix<T,2,2> H) {

        const auto [a, b, c, d] = H.to_flatten_array();
            // 使用更数值稳定的方法
        // const T trace = a + d;
        const T diff = a - d;
        const T off_diag = b + c;
        
        // 直接计算旋转角度，忽略次要项
        const T rotation_angle = atan2pu(off_diag, diff);
        
        
        return Rotation2<T>::from_angle(Angular<T>::from_turns(0.5f * rotation_angle));

        // // 直接计算旋转角度，忽略次要项
        // const auto rotation_angle = atan2pu(b + c, a - d);  // atan2(g, f)的2倍简化
        
        // return Rotation2<T>::from_angle(Angular<T>::from_turns(static_cast<T>(0.5f) * rotation_angle));

        const T e = static_cast<T>(0.5f) * (a + d);
        const T f = static_cast<T>(0.5f) * (a - d);
        const T g = static_cast<T>(0.5f) * (b + c);
        const T h = static_cast<T>(0.5f) * (c - b);

        // const T a1 = atan2pu(g, f);
        // const T a2 = atan2pu(h, e);

        // const auto theta_angle = Angular<T>::from_turns(static_cast<T>(0.5f) * (a2 - a1));
        // const auto phi_angle = Angular<T>::from_turns(static_cast<T>(0.5f) * (a2 + a1));

        // return Rotation2<T>::from_angle(theta_angle - phi_angle);  // ❌ 这里错了！
        // const T q = mag(e, h);
        // const T r = mag(f, g);

        // const T sx = q + r;
        // const T sy = q - r;

        // const auto sigma = Matrix<T,2,2>(
        //     sx, 0,
        //     0, sy
        // );

        const auto a1 = std::atan2(g, f);
        const auto a2 = std::atan2(h, e);

        const auto theta_angle = Angular<T>::from_radians(static_cast<T>(0.5f) * (a2 - a1));
        const auto phi_angle = Angular<T>::from_radians(static_cast<T>(0.5f) * (a2 + a1));

        // return Rotation2<T>::from_angle(theta_angle - phi_angle);

        const auto [theta_sin, theta_cos] = (theta_angle).sincos();
        const auto [phi_sin, phi_cos] = (phi_angle).sincos();

        const auto u = Matrix<T,2,2>(
            theta_cos, -theta_sin,
            theta_sin, theta_cos
        );

        const auto v = Matrix<T,2,2>(
            phi_cos, -phi_sin,
            phi_sin, phi_cos
        );

        return Rotation2<T>(u * v.transpose());
    }
}
    struct IndexRelations{
        struct Index{
            static constexpr uint16_t INVALID_NTH = 0xffff;
            static constexpr Index from_none(){
                return Index{INVALID_NTH};
            }

            static constexpr Index from_index(uint16_t nth){
                ASSERT(nth != INVALID_NTH);
                return Index{nth};
            }

            static constexpr Index from_index_unchecked(uint16_t nth){
                return Index{nth};
            }

            constexpr bool is_valid(Index index) const{
                return nth_ != INVALID_NTH;
            }

            constexpr uint16_t unwrap() const {
                if(nth_ == INVALID_NTH){
                    PANIC();
                }

                return nth_;
            }
        // private:
            uint16_t nth_;
        };

        struct IndexRelationsIterator{

        };

        // std::span<Index>
    };

    template<typename T>
    struct CorrespondPointCloudIterator{
    private:
        const std::span<const Vec2<T>> pts1_; 
        const std::span<const Vec2<T>> pts2_;
        IndexRelations relations_;
    };

    template<typename T>
    static constexpr Matrix2x2<T> compute_cross_variance_2x2(
        const std::span<const Vec2<T>> points1, 
        const std::span<const Vec2<T>> points2,
        const int16_t * cor
    ) {
        Matrix2x2<T> cov = Matrix2x2<T>::zero(); 
        for (size_t i = 0; i < points1.size(); i++) {
            const auto j = cor[i];
            if (j < 0) continue;
            const auto p = points1[i];
            const auto q = points2[j];
            cov.template at<0, 0>() += p.x * q.x;
            cov.template at<0, 1>() += p.y * q.x;
            cov.template at<1, 0>() += p.x * q.y;
            cov.template at<1, 1>() += p.y * q.y;
        }

        return cov;
    }
}


template<typename T, size_t D, 
    typename Isometry = std::conditional_t<D == 2, Isometry2<T>, Isometry3<T>>,
    typename Vec = std::conditional_t<D == 2, Vec2<T>, Vec3<T>>
>
static constexpr Result<Isometry, SlamErrorKind> pose_estimation(
    const std::span<const Vec> points1,
    const std::span<const Vec> points2,
    size_t max_iterations
){

    static_assert(D == 2 || D == 3, "Invalid dimension");

    using Rotation = typename Isometry::Rotation;
    using W_Matrix = Matrix<T, D, D>;

    if(points1.size() != points2.size())
        return Err(SlamErrorKind::PointsCntNotMatch);
    
    if(points1.size() < D)
        return Err(SlamErrorKind::InsufficientPoints);

    const auto n = points1.size();

    // 计算质心
    const auto centroid1 = std::reduce(points1.begin(), points1.end(), Vec::ZERO) / n;
    const auto centroid2 = std::reduce(points2.begin(), points2.end(), Vec::ZERO) / n;

    // 计算协方差矩阵 W
    const W_Matrix W = std::transform_reduce(
        points1.begin(), points1.end(), points2.begin(),
        Matrix<T, D, D>::zero(),
        
        std::plus<>(),
        [&](const Vec& p1, const Vec& p2) {
            return to_matrix(p1 - centroid1) * to_matrix(p2 - centroid2).transpose();
        }
    );

    #if 1
    // SVD on W: W = U * Sigma * V^T

    const auto may_sol = solve_jacobi_svd(W, max_iterations);
    if(may_sol.is_none())
        return Err(SlamErrorKind::JacobiSvdIterLimitReached);

    const auto & sol = may_sol.unwrap();
    const Matrix<T, D, D> & U = sol.U;
    const Matrix<T, D, D> & V = sol.V;

    
    // 确保是旋转矩阵（行列式为正）
    const auto R = Rotation::from_matrix(U * V.transpose());
    #else
    const auto R = slam::details::svd_2x2(W);
    #endif
    const auto t = centroid1 - R * centroid2;

    // DEBUG_PRINTLN("R", R, "t", t);
    return Ok(Isometry{
        .rotation = R, 
        .translation = t
    });
}


template<typename T, typename Fn>
static std::vector<Vec2<T>> make_points2d_from_lambda(Fn && fn, size_t num){
    std::vector<Vec2<T>> pts;
    pts.reserve(num);
    for(size_t i = 0; i < num; ++i){
        pts.push_back(std::forward<Fn>(fn)(i));
    }
    return pts;
}


template<typename T, typename Fn>
static auto make_point_cloud3d_from_lambda(Fn && fn, size_t num){
    std::vector<Vec3<T>> pts(num);
    for(size_t i = 0; i < num; ++i){
        pts.push_back(std::forward<Fn>(fn)(i));
    }
    return pts;
}

template<typename T>
static void point_cloud_demo(){
    static constexpr size_t NUM_POINTS = 400;
    static constexpr size_t MAX_ITERATIONS = 100;

    auto lambda1 = [](size_t i){ 
        return Vec2<T>::from_angle(Angular<T>::from_radians(i * static_cast<T>(0.1)));
    };

    auto lambda2 = [](size_t i){ 
        const auto angle = Angular<T>::from_radians(i * static_cast<T>(0.1) + 
            Angular<T>::from_degrees(10).to_radians());
        return Vec2<T>::from_angle(angle) + Vec2<T>(40, 50);
    };

    const auto points1 = make_points2d_from_lambda<T>(lambda1, NUM_POINTS);
    const auto points2 = make_points2d_from_lambda<T>(lambda2, NUM_POINTS);

    const auto begin_us = clock::micros();
    const auto res = pose_estimation<T, 2>(
        std::span(points1), 
        std::span(points2), 
        MAX_ITERATIONS
    );
    if(res.is_err()) PANIC(res.unwrap_err());
    DEBUG_PRINTLN(res.unwrap(), clock::micros() - begin_us);
}

void svd_main(){
    auto init_debugger = []{
        auto & DBG_UART = DEBUGGER_INST;

        DEBUGGER_INST.init({
            .remap = hal::USART2_REMAP_PA2_PA3,
            .baudrate = hal::NearestFreq(576_KHz), 
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(4);
        DEBUGGER.set_splitter(",");
        DEBUGGER.no_brackets(EN);
    };

    init_debugger();

    // point_cloud_demo<float>();
    point_cloud_demo<float>();

}

[[maybe_unused]] static void misc(){

    // Create a test matrix (3x2)
    const auto begin_us = clock::micros();
    auto A = Matrix<float, 3, 2>::from_uninitialized();
    A(0, 0) = 1.0; A(0, 1) = 2.0;
    A(1, 0) = 3.0; A(1, 1) = 4.0;
    A(2, 0) = 5.0; A(2, 1) = 6.0;

    
    // Compute SVD
    details::JacobiSVD<float, 3, 2> svd_solver(A, 100);  // 100 max iterations

    DEBUG_PRINTLN(clock::micros() - begin_us);
    
    // Get SVD components
    const auto sol = svd_solver.solution().expect();
    const auto U = sol.U;
    const auto sigma = sol.sigma;
    const auto V = sol.V;

    ASSERT(svd_solver.is_computed());
    DEBUG_PRINTLN(U);
    DEBUG_PRINTLN(U.transpose() * U);
    DEBUG_PRINTLN(sigma);
    DEBUG_PRINTLN(V);
    
    // Verify orthogonality of U (U^T * U should be identity)
    Matrix<float, 3, 3> U_orthogonality_check = U.transpose() * U;
    float u_error = 0.0;
    for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 3; j++) {
            float expected = (i == j) ? 1.0 : 0.0;
            u_error += math::square(U_orthogonality_check(i, j) - expected);
        }
    }
    
    // Verify orthogonality of V (V^T * V should be identity)
    Matrix<float, 2, 2> V_orthogonality_check = V.transpose() * V;
    float v_error = 0.0;
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 2; j++) {
            float expected = (i == j) ? 1.0 : 0.0;
            v_error += math::square(V_orthogonality_check(i, j) - expected);
        }
    }
    
    // Verify reconstruction: A ≈ U * Σ * V^T
    auto reconstructed = Matrix<float, 3, 2>::from_uninitialized();
    for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 2; j++) {
            reconstructed(i, j) = 0;
            for (size_t k = 0; k < 2; k++) {
                reconstructed(i, j) += U(i, k) * sigma(k, 0) * V(j, k);
            }
        }
    }
    
    float reconstruction_error = 0.0;
    for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 2; j++) {
            float diff = A(i, j) - reconstructed(i, j);
            reconstruction_error += diff * diff;
        }
    }
    reconstruction_error = std::sqrt(reconstruction_error);
    
    // Output results
    // DEBUGGER << "U matrix orthogonality error: " << std::sqrt(u_error) << std::endl;
    // DEBUGGER << "V matrix orthogonality error: " << std::sqrt(v_error) << std::endl;
    DEBUGGER << "U matrix orthogonality error: " << u_error * 0.001 << std::endl;
    DEBUGGER << "V matrix orthogonality error: " << v_error * 0.001 << std::endl;
    DEBUGGER << "Reconstruction error: " << reconstruction_error << std::endl;

}