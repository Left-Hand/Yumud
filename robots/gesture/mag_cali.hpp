#pragma once

#include "core/container/inline_vector.hpp"

#include "types/vectors/vector3.hpp"
#include "types/vectors/quat.hpp"

namespace ymd::robots{

// https://zhuanlan.zhihu.com/p/717479974

struct EllipseCalibrator_Prelude{

    struct Dignosis{
        Vector3<q24> center;
        Vector3<q24> soft_iron;
    };


    static constexpr Vector2<q24> project_idx_to_v2(const size_t i, const size_t n){
        const auto r = 1 - 2 * q16(i) / (n - 1);
        // q24 phi = std::cos(r); // 极角
        q24 phi = r; // 极角
        // q24 phi = 0;
        q24 theta = q24(M_PI) * (1.0_q24 + std::sqrt(5.0_q24)) * i; // 方位角
        return {phi, theta};
    }
    static constexpr Vector3<q24> project_v2_to_v3(Vector2<q24> v2){
        const auto [theta, phi] = v2;
        return {
            cos(theta) * sin(phi),
            sin(theta) * sin(phi),
            cos(phi)
        };
    }

    static constexpr Vector2<q24> project_v3_to_v2(const Vector3<q24> v3){
        q24 phi = std::acos(v3.z); // 极角
        q24 theta = std::atan2(v3.y, v3.x); // 方位角
        return {theta, phi};
    }

    static constexpr size_t project_v2_to_idx(const Vector2<q24> v2, const size_t n){
        const auto [theta, phi] = v2;
        q24 i = (1 - cos(phi)) * (n - 1) / 2; // 计算索引
        return static_cast<size_t>(i);
    }

    static constexpr Dignosis
    calibrate_magfield(const std::span<const Vector3<q24>> data) {
        // // 1. 计算平均值(初始硬铁偏移估计)

        const q24 inv_size = 1.0_q24 / data.size();


        // const Vector3<q24> center; = std::accumulate(data.begin(), data.end(), Vector3<q24>{0, 0, 0}) / data.size();
        //手动展开循环 避免溢出
        const auto center = [&]{
            Vector3<q24> sum{0, 0, 0};
            for (const auto & v : data) {
                sum += v * inv_size;
            }
            return sum;
        }();

        // 3. 构建线性方程组系数(避免矩阵运算)

        q24 A[6] = {0}; // 对称矩阵的上三角部分
        q24 b[3] = {0}; // 右侧向量

        for (size_t i = 0; i < data.size(); ++i) {
            const auto x = data[i].x - center.x;
            const auto y = data[i].y - center.y;
            const auto z = data[i].z - center.z;
            
            const auto x2 = (x * x);
            const auto y2 = (y * y);
            const auto z2 = (z * z);

            // 填充对称矩阵
            A[0] += x2 * x2; // A11
            A[1] += x2 * y2; // A12
            A[2] += x2 * z2; // A13
            A[3] += y2 * y2; // A22
            A[4] += y2 * z2; // A23
            A[5] += z2 * z2; // A33

            // 填充右侧向量
            b[0] += x2;
            b[1] += y2;
            b[2] += z2;

            DEBUG_PRINTLN(A, b);
            clock::delay(1ms);
        }


        // 4. 解3x3线性方程组(使用克莱姆法则，避免矩阵求逆)
        auto solve_3x3 = [&]() -> Vector3<q24> {
            // 计算行列式
            const auto det = A[0]*(A[3]*A[5] - A[4]*A[4]) 
                            - A[1]*(A[1]*A[5] - A[4]*A[2]) 
                            + A[2]*(A[1]*A[4] - A[3]*A[2]);

            if (det == 0) {
                return Vector3<q24>{1, 1, 1};
            }

            // 计算各变量的行列式
            const auto det_x = b[0]*(A[3]*A[5] - A[4]*A[4]) 
                                - A[1]*(b[1]*A[5] - A[4]*b[2]) 
                                + A[2]*(b[1]*A[4] - A[3]*b[2]);

            const auto det_y = A[0]*(b[1]*A[5] - A[4]*b[2]) 
                                - b[0]*(A[1]*A[5] - A[4]*A[2]) 
                                + A[2]*(A[1]*b[2] - b[1]*A[2]);

            const auto det_z = A[0]*(A[3]*b[2] - b[1]*A[4]) 
                                - A[1]*(A[1]*b[2] - b[1]*A[2]) 
                                + b[0]*(A[1]*A[4] - A[3]*A[2]);

            return Vector3<q24>{
                det_x / det,
                det_y / det,
                det_z / det
            };
        };

        Vector3<q24> solution = solve_3x3();

        // 5. 计算软铁缩放因子
        auto compute_scale_factors = [](Vector3<q24> params) {
            return Vector3<q24>{
                isqrt(MAX(params.x, 1)),
                isqrt(MAX(params.y, 1)),
                isqrt(MAX(params.z, 1))
            };
        };

        Vector3<q24> soft_iron = compute_scale_factors(solution);

        return {
            .center = center, 
            .soft_iron = soft_iron
        };
    }
};

class EllipseCalibrator:public EllipseCalibrator_Prelude{
public:
    static constexpr size_t N = 48;
    // using Data = std::array<Vector3<q24>, N>;
    using Data = InlineVector<Vector3<q24>, N>;

    // struct Flag{
        // Empty,
        // Valid
    // };
    using Flag = uint8_t;

    using Flags = std::array<Flag, N>;

    EllipseCalibrator(){
        flags_.fill(0);
    }

    void add_data(const Vector3<q24> & v3){
        // const auto v2 = project_v3_to_v2(v3.normalized());
        // const auto idx = project_v2_to_idx(v2, N);

        const auto idx8 = [&] -> uint8_t{
            const auto [x,y,z] = v3;
            const uint8_t b1 = std::signbit(x);
            const uint8_t b2 = std::signbit(y);
            const uint8_t b3 = std::signbit(z);
            return b1 << 2 | b2 << 1 | b3;
        }();

        const auto idx6 = [&] -> uint8_t{
            const auto [x0,y0,z0] = v3;
            const auto [x,y,z] = Vector3{ABS(x0), ABS(y0), ABS(z0)};

            const bool b1 = std::signbit(y-z);
            const bool b2 = std::signbit(x-z);
            const bool b3 = std::signbit(x-y);
            switch(b1 << 2 | b2 << 1 | b3){
                case 0b100: return 0;
                case 0b110: return 1;
                case 0b111: return 2;
                case 0b011: return 3;
                case 0b001: return 4;
                case 0b000: return 5;
                default: PANIC(b1, b2, b3);
            }
        }();

        const size_t idx = idx8 * 6 + idx6;
        // if(idx >= N) PANIC(N);
        if(idx < N and is_index_empty(idx)){
            // data_[idx] = v3;
            data_.push_back(v3);
            flags_[idx] = 1;
        }
    }


    auto dignosis() const {
        return calibrate_magfield(std::span(data_));
    }

    template<typename T>
    static constexpr T reduce_span(const std::span<const T> data){
            return std::accumulate(data.begin(), data.end(), 0);
    };

    std::array<uint8_t, 8> get_progress() const {
        std::array<uint8_t, 8> ret;

        for(size_t i = 0; i < 48; i+=6){
            ret[i / 6] = reduce_span(std::span(flags_.begin() + i, 6));
        }
        
        return ret;
    }

    uint8_t get_percentage() const {
        const auto prog = get_progress();
        uint8_t sum = 0;
        for(auto v : prog) sum += v;
        return sum;
        // return std::accumulate(prog.begin(), prog.end(), uint8_t(0)) * 100 / 48;
    }


    bool is_index_empty(size_t i){
        return flags_[i] == false;
    }

    std::span<const Vector3<q24>> data() const {return data_;}
private:
    Data data_;
    Flags flags_;

};

}