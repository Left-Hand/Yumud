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

template<typename T>
struct Rotation3{
    // Matrix<T, 3, 3> matrix;



    static constexpr Rotation3 from_identity(){
        return Rotation3(Quat<T>::from_identity());
    }

    static constexpr Rotation3 from_matrix(const Matrix<T, 3, 3> & matrix){
        return Rotation3(mat3x3_to_quat(matrix));
    }

    constexpr Vec3<T> operator *(const Vec3<T>& v) const {
        return Vec3<T>(quat_ * to_matrix(v));
    }


private:
    Quat<T> quat_;
};

template<typename T>
struct Rotation2 {
    constexpr explicit Rotation2(const Matrix2x2<T>& matrix):
        sine_(matrix.template at<1,0>()), cosine_(matrix.template at<1,1>()){;} 

    // 从角度构造
    [[nodiscard]] static constexpr 
    Rotation2 from_angle(Angle<T> angle) {
        const auto [s, c] = angle.sincos();
        return Rotation2{s, c};
    }


    // 恒等旋转（单位旋转）
    [[nodiscard]] static constexpr 
    Rotation2 from_identity() {
        return Rotation2{T(0), T(1)};
    }
    
    // 从旋转矩阵构造（如果矩阵是标准形式）
    [[nodiscard]] static constexpr 
    Rotation2 from_matrix(const Matrix<T, 2, 2>& mat) {
        // 假设矩阵形式为：[cos, -sin]
        //                [sin,  cos]
        return Rotation2{mat(1, 0), mat(0, 0)};
    }

    [[nodiscard]] constexpr 
    Matrix<T, 2, 2> to_matrix() const {
        return Matrix<T, 2, 2>(
            cosine_, -sine_,
            sine_,  cosine_
        );
    }

    [[nodiscard]] constexpr 
    Angle<T> angle() const {
        return Angle<T>::from_turns(atan2pu(sine_, cosine_));
    }

    [[nodiscard]] constexpr 
    Vec2<T> operator*(const Vec2<T>& v) const {
        return Vec2<T>(
            cosine_ * v.x - sine_ * v.y,
            sine_ * v.x + cosine_ * v.y
        );
    }

    [[nodiscard]] constexpr 
    Rotation2 operator*(const Rotation2& other) const {
        // 三角函数公式：sin(a+b) = sin(a)cos(b) + cos(a)sin(b)
        //             cos(a+b) = cos(a)cos(b) - sin(a)sin(b)
        return Rotation2{
            sine_ * other.cosine_ + cosine_ * other.sine_,
            cosine_ * other.cosine_ - sine_ * other.sine_
        };
    }

    // 逆旋转（转置）
    [[nodiscard]] constexpr 
    Rotation2 inverse() const {
        return Rotation2{-sine_, cosine_}; // 角度取反
    }

    [[nodiscard]] constexpr 
    Rotation2 transpose() const {
        return inverse();
    }

    [[nodiscard]] constexpr T sine() const {return sine_;}
    [[nodiscard]] constexpr T cosine() const {return cosine_;}

    #if 0
    // 插值：球面线性插值
    constexpr Rotation2 slerp(const Rotation2& other, T t) const {
        T cos_theta = cosine_ * other.cosine_ + sine_ * other.sine_;
        
        // 处理数值误差
        cos_theta = std::clamp(cos_theta, T(-1), T(1));
        
        T theta = std::acos(cos_theta);
        
        if (std::abs(theta) < 1e-6) {
            // 角度很小，直接线性插值
            return Rotation2{
                sine_ + t * (other.sine_ - sine_),
                cosine_ + t * (other.cosine_ - cosine_)
            }.normalized();
        }
        
        T sin_theta = std::sin(theta);
        T a = std::sin((1 - t) * theta) / sin_theta;
        T b = std::sin(t * theta) / sin_theta;
        
        return Rotation2{
            a * sine_ + b * other.sine_,
            a * cosine_ + b * other.cosine_
        }.normalized();
    }
    #endif


private:
    T sine_;
    T cosine_;


    // 直接从sine和cosine构造
    constexpr Rotation2(T sin_val, T cos_val) : sine_(sin_val), cosine_(cos_val) {;}

    friend OutputStream& operator<<(OutputStream& os, const Rotation2<T>& self) {
        return os << "Rotation2(sin=" << self.sine_ << ", cos=" << self.cosine_ 
                    << ", angle=" << self.angle().to_degrees() << "°)";
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

    friend OutputStream & operator << (OutputStream & os, const Isometry3<T> & self) { 
        return os << "rotation: " << self.rotation << " translation: " << self.translation;
    }
};

template<typename T>
struct Isometry2 { 
    using Rotation = Rotation2<T>;
    using Vec = Vec2<T>;

    Rotation2<T> rotation;
    Vec2<T> translation;

    friend OutputStream & operator << (OutputStream & os, const Isometry2<T> & self) { 
        return os << "rotation: " << self.rotation << " translation: " << self.translation;
    }
};

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
    
    // centroid of mass
    auto centroid_of_pts = [](const std::span<const Vec> pts) -> Vec {
        Vec sum = Vec::ZERO;
        for (auto &pt : pts)
            sum += pt;
        return sum / static_cast<T>(pts.size());
    };

    auto  covariance_mat_of_vec = [](const Vec centroid1, const Vec centroid2) -> W_Matrix {
        #if 0

        auto ret = W_Matrix::from_uninitialized();
        ret(0, 0) = centroid1.x * centroid2.x;
        ret(0, 1) = centroid1.x * centroid2.y;
        ret(1, 0) = centroid1.y * centroid2.x;
        ret(1, 1) = centroid1.y * centroid2.y;
        return ret;

        #else

        return to_matrix(centroid1) * to_matrix(centroid2).transpose();

        #endif
    };

    const Vec centroid1 = centroid_of_pts(pts1);
    const Vec centroid2 = centroid_of_pts(pts2);


    // Compute covariance matrix W
    const auto W = [&]{
        auto ret = W_Matrix::from_zero();
        for (size_t i = 0; i < n; i++) {
            ret += covariance_mat_of_vec(pts1[i] - centroid1, pts2[i] - centroid2);
        }
        return ret;
    }();

    #if 1
    // SVD on W: W = U * Sigma * V^T
    const JacobiSVD<T, D, D> svd_solver(W, max_iterations);

    if(!svd_solver.is_computed()) 
        return Err(SlamErrorKind::JacobiSvdIterLimitReached);

    const auto sol = svd_solver.solution().unwrap();
    const Matrix<T, D, D> & U = sol.U;
    const Matrix<T, D, D> & V = sol.V;

    
    // 确保是旋转矩阵（行列式为正）
    auto R_matrix = U * V.transpose();
    // DEBUG_PRINTLN("u", U, "v", V, V.transpose(), U * V.transpose());
    const auto R = Rotation(R_matrix);
    #else
    const auto R = slam::details::svd_2x2(W);
    #endif
    const auto t = centroid1 - R * centroid2;  // 直接使用Rotation3的运算符

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
    JacobiSVD<float, 3, 2> svd_solver(A, 100);  // 100 max iterations

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