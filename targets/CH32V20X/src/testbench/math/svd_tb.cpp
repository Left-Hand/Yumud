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
#include "types/vectors/Vector3.hpp"

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
struct ToMatrixDispatcher{
    //static constexpr auto cast();
};

template<typename T>
struct ToMatrixDispatcher<Vec3<T>>{
    static constexpr auto cast(const Vec3<T>& p){
        return Matrix<T, 3, 1>(p.x, p.y, p.z);
    }
};


template<typename T>
struct ToMatrixDispatcher<Vec2<T>>{
    static constexpr auto cast(const Vec2<T>& p){
        return Matrix<T, 2, 1>(p.x, p.y);
    }
};


template<typename T>
auto to_matrix(T && obj){
    return ToMatrixDispatcher<std::decay_t<T>>::cast(obj);
}

template<typename T>
struct Rotation3{
    Matrix<T, 3, 3> matrix;

    static constexpr Rotation3 from_identity(){
        return Rotation3(Matrix<T, 3, 3>::identity());
    }

    static constexpr Rotation3 from_matrix(const Matrix<T, 3, 3> & matrix){
        return Rotation3(matrix);
    }

    constexpr Vec3<T> operator *(const Vec3<T>& v) const {
        return Vec3<T>(matrix * to_matrix(v));
    }

    constexpr Rotation3 normalized() const { 
        return *this;
    }

    friend OutputStream & operator << (OutputStream & os, const Rotation3<T> & self) { 
        return os << self.matrix;
    }

};

template<typename T>
struct Rotation2{
    Matrix<T, 2, 2> matrix;

    static constexpr Rotation2 from_angle(Angle<T> angle){
        const auto [s,c] = angle.sincos();
        const auto matrix = Matrix<T, 2, 2>(
            c, s,
            -s, c
        );

        return Rotation2{matrix};
    }

    constexpr Rotation2 normalized() const { 
        return *this;
    }

    constexpr Vec2<T> operator *(const Vec2<T>& v) const {
        return Vec2<T>(matrix * to_matrix(v));
    }

    friend OutputStream & operator << (OutputStream & os, const Rotation2<T> & self) { 
        return os << self.matrix;
    }
};

template<typename T>
struct Isometry3 { 
    using Rotation = Rotation3<T>;
    using Vec = Vec3<T>;

    Rotation3<T> rotation;
    Vec3<T> translation;

    // constexpr Matrix<T, 4, 4> to_matrix() const { 

    // }

    friend OutputStream & operator << (OutputStream & os, const Isometry3<T> & iso) { 
        return os << "rotation: " << iso.rotation << " translation: " << iso.translation;
    }
};

template<typename T>
struct Isometry2 { 
    using Rotation = Rotation2<T>;
    using Vec = Vec2<T>;

    Rotation2<T> rotation;
    Vec2<T> translation;

    friend OutputStream & operator << (OutputStream & os, const Isometry2<T> & iso) { 
        return os << "rotation: " << iso.rotation << " translation: " << iso.translation;
    }
};

template<typename T, size_t D, 
    typename Isometry = std::conditional_t<D == 2, Isometry2<T>, Isometry3<T>>,
    typename Vec = std::conditional_t<D == 2, Vec2<T>, Vec3<T>>
>
static Result<Isometry, SlamErrorKind> pose_estimation(
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
    
    // center of mass
    auto center_of_pts = [](const std::span<const Vec> pts) -> Vec {
        Vec sum = Vec::ZERO;
        for (auto &pt : pts)
            sum += pt;
        return sum / static_cast<T>(pts.size());
    };

    auto  covariance_mat_of_vec = [](const Vec p1, const Vec p2) -> W_Matrix {
        auto ret = W_Matrix::from_uninitialized();
        ret(0, 0) = p1.x * p2.x;
        ret(0, 1) = p1.x * p2.y;
        ret(1, 0) = p1.y * p2.x;
        ret(1, 1) = p1.y * p2.y;
        return ret;
        // return to_matrix(p1) * to_matrix(p2).transpose();
    };

    const Vec p1 = center_of_pts(pts1);
    const Vec p2 = center_of_pts(pts2);

    // DEBUG_PRINTLN(p1, p2);

    // Compute covariance matrix W
    auto W = W_Matrix::from_zero();
    for (size_t i = 0; i < n; i++) {
        W += covariance_mat_of_vec(pts1[i] - p1, pts2[i] - p2);
    }

    // SVD on W: W = U * Sigma * V^T
    JacobiSVD<T, D, D> svd(W, max_iterations);

    if(!svd.is_computed()) 
        return Err(SlamErrorKind::JacobiSvdIterLimitReached);

    const auto sol = svd.solution().unwrap();
    const Matrix<T, D, D> & U = sol.U;
    const Matrix<T, D, D> & V = sol.V;

    
    // 确保是旋转矩阵（行列式为正）
    auto R_matrix = U * V.transpose();
    // DEBUG_PRINTLN("u", U, "v", V, V.transpose(), U * V.transpose());

    const auto R = Rotation(R_matrix).normalized();
    const auto t = p1 - R * p2;  // 直接使用Rotation3的运算符

    // DEBUG_PRINTLN("R", R, "t", t);
    return Ok(Isometry(R, t));
}


template<typename T, typename Fn>
static auto make_point_cloud2d_from_lambda(Fn && fn, size_t N){
    std::vector<Vec2<T>> pts;
    pts.reserve(N);
    for(size_t i = 0; i < N; ++i){
        pts.push_back(std::forward<Fn>(fn)(i));
    }
    return pts;
}


template<typename T, typename Fn>
static auto make_point_cloud3d_from_lambda(Fn && fn, size_t N){
    std::vector<Vec3<T>> pts(N);
    for(size_t i = 0; i < N; ++i){
        pts.push_back(std::forward<Fn>(fn)(i));
    }
    return pts;
}

template<typename T>
static void point_cloud_demo(){
    static constexpr size_t N = 400;
    static constexpr size_t MAX_ITERATIONS = 100;

    auto lambda1 = [](size_t i){ 
        return Vec2<T>::from_angle(Angle<T>::from_radians(i * static_cast<T>(0.1)));
    };

    auto lambda2 = [](size_t i){ 
        const auto angle = Angle<T>::from_radians(i * static_cast<T>(0.1) + static_cast<T>(0.1));
        return Vec2<T>::from_angle(angle) + Vec2<T>(40, 50);
    };

    const auto pts1 = make_point_cloud2d_from_lambda<T>(lambda1,N);
    const auto pts2 = make_point_cloud2d_from_lambda<T>(lambda2,N);

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
    JacobiSVD<float, 3, 2> svd(A, 100);  // 100 max iterations

    DEBUG_PRINTLN(clock::micros() - begin_micros);
    
    // Get SVD components
    const auto sol = svd.solution().expect();
    const auto U = sol.U;
    const auto sigma = sol.sigma;
    const auto V = sol.V;

    ASSERT(svd.is_computed());
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