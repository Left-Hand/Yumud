#pragma once

#include "types/vectors/vector2/Vector2.hpp"
#include "types/matrix/matrix_static.hpp"

namespace ymd{


template<typename T>
struct PerspectiveRect{

    std::array<Vector2<T>, 4> points;
    

    // 从uint8坐标构造（自动归一化到[0,1]范围）
    constexpr PerspectiveRect(std::array<Vector2<uint8_t>, 4> u8points) {
        for (size_t i = 0; i < 4; ++i) {
            points[i] = Vector2<T>{
                u8points[i].x * T(1.0 / 255),
                u8points[i].y * T(1.0 / 255)
            };
        }
    }


    // 从uint8坐标构造（自动归一化到[0,1]范围）
    constexpr PerspectiveRect(std::array<std::tuple<uint8_t, uint8_t>, 4> u8points) {
        for (size_t i = 0; i < 4; ++i) {
            points[i] = Vector2<T>{
                std::get<0>(u8points[i]) * T(1.0 / 255),
                std::get<1>(u8points[i]) * T(1.0 / 255)
            };
        }
    }

    constexpr auto to_u8points() const -> std::array<std::tuple<uint8_t, uint8_t>, 4> { 
        return std::array<std::tuple<uint8_t, uint8_t>, 4>{
            std::make_tuple(uint8_t(points[0].x * 255), uint8_t(points[0].y * 255)),
            std::make_tuple(uint8_t(points[1].x * 255), uint8_t(points[1].y * 255)),
            std::make_tuple(uint8_t(points[2].x * 255), uint8_t(points[2].y * 255)),
            std::make_tuple(uint8_t(points[3].x * 255), uint8_t(points[3].y * 255)),
        };
    }

    constexpr auto to_u8x8() const -> std::array<uint8_t, 8>{
        std::array<uint8_t, 8> result;
        for (size_t i = 0; i < 4; ++i) {
            result[i * 2] = uint8_t(points[i].x * 255);
            result[i * 2 + 1] = uint8_t(points[i].y * 255);
        }
        return result;
    }

    static constexpr auto from_u8x8(const std::span<const uint8_t, 8> u8x8) -> PerspectiveRect { 
        std::array<Vector2<uint8_t>, 4> u8points;
        for (size_t i = 0; i < 4; ++i) {
            u8points[i] = Vector2<uint8_t>(u8x8[i * 2], u8x8[i * 2 + 1]);
        }
        return PerspectiveRect(u8points);
    }

    // 添加以下函数到 PerspectiveRect 结构体中
    static constexpr PerspectiveRect from_clockwise_points(
        std::array<Vector2<T>, 4> f32points) {
        // 如果点不是顺时针排列，则交换它们以确保顺时针顺序
        // 检查前三个点的叉积来判断方向
        auto cross = [](const Vector2<T>& a, const Vector2<T>& b, const Vector2<T>& c) {
            return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        };
        
        // 检查第一个三个点的方向
        T cr = cross(f32points[0], f32points[1], f32points[2]);
        
        // 如果是逆时针，则反转数组（除了第一个点）
        if (cr > 0) {
            return PerspectiveRect(std::array<Vector2<T>, 4>{
                f32points[0], f32points[3], f32points[2], f32points[1]
            });
        }
        
        return PerspectiveRect(f32points);
    }

    // 从浮点坐标直接构造
    constexpr PerspectiveRect(std::array<Vector2<T>, 4> f32points)
        : points(f32points) {}

    // 计算透视中心点（两条对角线的交点）
    constexpr Vector2<T> center() const {
        // 对于四边形，中心点应该是两条对角线的交点
        // 对角线1: 从 points[0] 到 points[2]
        // 对角线2: 从 points[1] 到 points[3]
        
        const Vector2<T>& p0 = points[0];
        const Vector2<T>& p1 = points[1];
        const Vector2<T>& p2 = points[2];
        const Vector2<T>& p3 = points[3];
        
        // 计算两条对角线的交点
        // 对角线1: P0 + t1 * (P2 - P0)
        // 对角线2: P1 + t2 * (P3 - P1)
        
        T denominator = (p2.x - p0.x) * (p3.y - p1.y) - (p2.y - p0.y) * (p3.x - p1.x);
        
        if (std::abs(denominator) < T(1e-5)) {
            // 如果分母接近0，说明是平行线，退化为简单平均
            return Vector2<T>{(p0.x + p1.x + p2.x + p3.x) * T(0.25),
                            (p0.y + p1.y + p2.y + p3.y) * T(0.25)};
        }
        
        T t = ((p1.x - p0.x) * (p3.y - p1.y) - (p1.y - p0.y) * (p3.x - p1.x)) / denominator;
        
        // 返回对角线1上的点作为中心点
        return Vector2<T>{p0.x + t * (p2.x - p0.x),
                        p0.y + t * (p2.y - p0.y)};
    }

    // 计算面积（使用鞋带公式）
    constexpr T area() const {
        T a = 0;
        for (size_t i = 0; i < 4; ++i) {
            size_t j = (i + 1) % 4;
            a += points[i].x * points[j].y - points[j].x * points[i].y;
        }
        return std::abs(a) * T(0.5f);
    }

    // 平移变换
    constexpr PerspectiveRect shifted(Vector2<T> offset) const {
        std::array<Vector2<T>, 4> new_points;
        for (size_t i = 0; i < 4; ++i) {
            new_points[i] = points[i] + offset;
        }
        return PerspectiveRect(new_points);
    }

    // 缩放变换（以中心点为原点）
    constexpr PerspectiveRect scaled(T factor) const {
        const auto c = center();
        std::array<Vector2<T>, 4> new_points;
        for (size_t i = 0; i < 4; ++i) {
            new_points[i] = c + (points[i] - c) * factor;
        }
        return PerspectiveRect(new_points);
    }

    // 检查是否是凸四边形
    constexpr bool is_convex() const {
        auto cross = [](Vector2<T> a, Vector2<T> b, Vector2<T> c) {
            return (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
        };

        bool sign = false;
        for (size_t i = 0; i < 4; ++i) {
            size_t a = i;
            size_t b = (i + 1) % 4;
            size_t c = (i + 2) % 4;
            T cr = cross(points[a], points[b], points[c]);
            if (i == 0) {
                sign = cr > 0;
            } else if ((cr > 0) != sign) {
                return false;
            }
        }
        return true;
    }


    // 计算单应性矩阵 H（3x3）
    constexpr Matrix<T, 3, 3> compute_homography() const {
        // 源点（单位正方形）
        constexpr std::array<Vector2<T>, 4> src = {
            Vector2<T>{0, 0},
            Vector2<T>{1, 0},
            Vector2<T>{1, 1},
            Vector2<T>{0, 1}
        };

        // 构造线性方程组 A * h = b（8x8）
        std::array<std::array<T, 9>, 8> augmented{};

        for (size_t i = 0; i < 4; ++i) {
            T u = src[i].x;
            T v = src[i].y;
            T x = points[i].x;
            T y = points[i].y;

            // 方程 1: h11*u + h12*v + h13 - x*(h31*u + h32*v) = x
            augmented[2*i] = {
                u, v, 1, 0, 0, 0, -x*u, -x*v, x
            };

            // 方程 2: h21*u + h22*v + h23 - y*(h31*u + h32*v) = y
            augmented[2*i + 1] = {
                0, 0, 0, u, v, 1, -y*u, -y*v, y
            };
        }

        // 高斯消元法（constexpr）
        constexpr auto solve = [](auto& mat) {
            constexpr size_t N = 8;
            for (size_t col = 0; col < N; ++col) {
                // 找主元
                size_t max_row = col;
                for (size_t row = col + 1; row < N; ++row) {
                    if (std::abs(mat[row][col]) > std::abs(mat[max_row][col])) {
                        max_row = row;
                    }
                }

                // 交换行
                if (max_row != col) {
                    std::swap(mat[col], mat[max_row]);
                }

                // 消元
                for (size_t row = col + 1; row < N; ++row) {
                    T factor = mat[row][col] / mat[col][col];
                    for (size_t k = col; k <= N; ++k) {
                        mat[row][k] -= factor * mat[col][k];
                    }
                }
            }

            // 回代
            std::array<T, N> h{};

            for (size_t row = N; row > 0; --row) {
                size_t i = row - 1;
                // Use 'i' instead of 'row' in the loop body
                h[i] = mat[i][N];
                for (size_t col = i + 1; col < N; ++col) {
                    h[i] -= mat[i][col] * h[col];
                }
                h[i] /= mat[i][i];
            }
            return h;
        };

        // 解方程组
        auto h = solve(augmented);

        // 构造 3x3 单应性矩阵
        return Matrix<T, 3, 3>(
            h[0], h[1], h[2],
            h[3], h[4], h[5],
            h[6], h[7], 1
        );
    }



    friend OutputStream & operator << (OutputStream & os, const PerspectiveRect & rect){
        return os << rect.points;
    }
};


// 映射 (u, v) -> (x, y)

template<typename T>
static constexpr Vector2<T> map_uv(const Matrix<T, 3, 3> & H, Vector2<T> uv){
    // const auto H = compute_homography();
    const T u = uv.x;
    const T v = uv.y;

    // 计算 H * [u, v, 1]^T
    const T x_prime = H[0][0] * u + H[0][1] * v + H[0][2];
    const T y_prime = H[1][0] * u + H[1][1] * v + H[1][2];
    const T w_prime = H[2][0] * u + H[2][1] * v + H[2][2];

    // 归一化
    return Vector2<T>{
        x_prime / w_prime,
        y_prime / w_prime
    };
}


}
