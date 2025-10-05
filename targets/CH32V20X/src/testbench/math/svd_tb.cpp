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
#include "types/vectors/vector3.hpp"
#include "types/vectors/quat.hpp"

#include "types/gesture/isometry2.hpp"
#include "types/gesture/isometry3.hpp"

#include "core/string/string_view.hpp"

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


template<typename T>
static constexpr Quat<T> mat3x3_to_quat(const Matrix<T, 3, 3>& R){
    // https://zhuanlan.zhihu.com/p/635847061
    const T trace = R(0, 0) + R(1, 1) + R(2, 2);
    std::array<T, 4> buf;

    if (trace >= 0.0) {
        T t = sqrt(trace + T(1.0));
        buf[0] = T(0.5) * t;
        t = T(0.5) / t;
        buf[1] = (R(2, 1) - R(1, 2)) * t;
        buf[2] = (R(0, 2) - R(2, 0)) * t;
        buf[3] = (R(1, 0) - R(0, 1)) * t;
    } else {
        size_t i = 0;
        
        if (R(1, 1) > R(0, 0)) {
            i = 1;
        }

        if (R(2, 2) > R(i, i)) {
            i = 2;
        }

        const size_t j = (i + 1) % 3;
        const size_t k = (j + 1) % 3;
        T t = sqrt(R(i, i) - R(j, j) - R(k, k) + T(1.0));
        buf[i + 1] = T(0.5) * t;
        t = T(0.5) / t;
        buf[0] = (R(k, j) - R(j, k)) * t;
        buf[j + 1] = (R(j, i) + R(i, j)) * t;
        buf[k + 1] = (R(k, i) + R(i, k)) * t;
    }

    return Quat<T>::from_array(buf);
}

template<typename T>
static constexpr Matrix3x3<T> quat_to_mat3x3(const Quat<T> q){
    // https://zhuanlan.zhihu.com/p/635847061
    const auto [x, y, z, w] = q.to_xyzw_array();
    return Matrix3x3<T>(
        1 - 2 * (y * y + z * z),            2 * (x * y - z * w),            2 * (x * z + y * w),
        2 * (x * y + z * w),                1 - 2 * (x * x + z * z),        2 * (y * z - x * w),
        2 * (x * z - y * w),                2 * (y * z + x * w),            1 - 2 * (x * x + y * y)
    );
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
        
        
        return Rotation2<T>::from_angle(Angle<T>::from_turns(0.5f * rotation_angle));

        // // 直接计算旋转角度，忽略次要项
        // const auto rotation_angle = atan2pu(b + c, a - d);  // atan2(g, f)的2倍简化
        
        // return Rotation2<T>::from_angle(Angle<T>::from_turns(static_cast<T>(0.5f) * rotation_angle));

        const T e = static_cast<T>(0.5f) * (a + d);
        const T f = static_cast<T>(0.5f) * (a - d);
        const T g = static_cast<T>(0.5f) * (b + c);
        const T h = static_cast<T>(0.5f) * (c - b);

        // const T a1 = atan2pu(g, f);
        // const T a2 = atan2pu(h, e);

        // const auto theta_angle = Angle<T>::from_turns(static_cast<T>(0.5f) * (a2 - a1));
        // const auto phi_angle = Angle<T>::from_turns(static_cast<T>(0.5f) * (a2 + a1));

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

        const auto theta_angle = Angle<T>::from_radians(static_cast<T>(0.5f) * (a2 - a1));
        const auto phi_angle = Angle<T>::from_radians(static_cast<T>(0.5f) * (a2 + a1));

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
        const std::span<const Vec2<T>> pts1, 
        const std::span<const Vec2<T>> pts2,
        const int16_t * cor
    ) {
        Matrix2x2<T> cov = Matrix2x2<T>::from_zero(); 
        for (size_t i = 0; i < pts1.size(); i++) {
            const auto j = cor[i];
            if (j < 0) continue;
            const auto p = pts1[i];
            const auto q = pts2[j];
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
    const std::span<const Vec> pts1,
    const std::span<const Vec> pts2,
    size_t max_iterations
){

    static_assert(D == 2 || D == 3, "Invalid dimension");

    using Rotation = typename Isometry::Rotation;
    using W_Matrix = Matrix<T, D, D>;

    if(pts1.size() != pts2.size())
        return Err(SlamErrorKind::PointsCntNotMatch);
    
    if(pts1.size() < D)
        return Err(SlamErrorKind::InsufficientPoints);

    const auto n = pts1.size();

    // 计算质心
    const auto centroid1 = std::reduce(pts1.begin(), pts1.end(), Vec::ZERO) / n;
    const auto centroid2 = std::reduce(pts2.begin(), pts2.end(), Vec::ZERO) / n;

    // 计算协方差矩阵 W
    const W_Matrix W = std::transform_reduce(
        pts1.begin(), pts1.end(), pts2.begin(),
        Matrix<T, D, D>::from_zero(),
        
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
        return Vec2<T>::from_angle(Angle<T>::from_radians(i * static_cast<T>(0.1)));
    };

    auto lambda2 = [](size_t i){ 
        const auto angle = Angle<T>::from_radians(i * static_cast<T>(0.1) + 
            Angle<T>::from_degrees(10).to_radians());
        return Vec2<T>::from_angle(angle) + Vec2<T>(40, 50);
    };

    const auto pts1 = make_points2d_from_lambda<T>(lambda1,NUM_POINTS);
    const auto pts2 = make_points2d_from_lambda<T>(lambda2,NUM_POINTS);

    const auto begin_micros = clock::micros();
    const auto res = pose_estimation<T, 2>(std::span(pts1), std::span(pts2), MAX_ITERATIONS);
    if(res.is_err()) PANIC(res.unwrap_err());
    DEBUG_PRINTLN(res.unwrap(), clock::micros() - begin_micros);
}

void svd_main(){
    auto init_debugger = []{
        auto & DBG_UART = DEBUGGER_INST;

        DBG_UART.init({
            .baudrate = UART_BAUD
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
    const auto begin_micros = clock::micros();
    auto A = Matrix<float, 3, 2>::from_uninitialized();
    A(0, 0) = 1.0; A(0, 1) = 2.0;
    A(1, 0) = 3.0; A(1, 1) = 4.0;
    A(2, 0) = 5.0; A(2, 1) = 6.0;

    // PANIC{A};
    
    // Compute SVD
    details::JacobiSVD<float, 3, 2> svd_solver(A, 100);  // 100 max iterations

    DEBUG_PRINTLN(clock::micros() - begin_micros);
    
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
            u_error += square(U_orthogonality_check(i, j) - expected);
        }
    }
    
    // Verify orthogonality of V (V^T * V should be identity)
    Matrix<float, 2, 2> V_orthogonality_check = V.transpose() * V;
    float v_error = 0.0;
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 2; j++) {
            float expected = (i == j) ? 1.0 : 0.0;
            v_error += square(V_orthogonality_check(i, j) - expected);
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