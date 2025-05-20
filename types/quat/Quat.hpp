#pragma once

/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/* Copyright (c) 2024  Rstr1aN / Yumud                                    */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/*                                                                        */
/* Note: This file has been modified by Rstr1aN / Yumud.                  */
/**************************************************************************/


// #include "vector3/vector3.hpp"
#include "types/vector3/vector3.hpp"
#include "types/euler/euler.hpp"
// #include "core/math/fast/conv.hpp"

namespace ymd{

template <arithmetic T>

struct Quat_t{
    T x;
    T y;
    T z;
    T w;

static_assert(not std::is_integral_v<T>);

    __fast_inline constexpr Quat_t() :
            x(0),
            y(0),
            z(0),
            w(1) {
    }

    static constexpr Quat_t<T> IDENTITY = Quat_t<T>(0,0,0,1);

    [[nodiscard]]

    __fast_inline constexpr Quat_t(const auto p_x, const auto p_y, const auto p_z, const auto p_w) :
            x(static_cast<T>(p_x)),
            y(static_cast<T>(p_y)),
            z(static_cast<T>(p_z)),
            w(static_cast<T>(p_w)) {
    }

    [[nodiscard]]
    constexpr Quat_t(const Vector3_t<T> &axis, const T &angle) { set_axis_angle(axis, angle); }

    [[nodiscard]]
    static constexpr Quat_t from_shortest_arc(const Vector3_t<T> &v0, const Vector3_t<T> &v1){
        const Vector3_t<T> n0 = v0.normalized();
        const Vector3_t<T> n1 = v1.normalized();

        T d = v0.dot(v1);

        if (std::abs(d) > T(1) - T(CMP_EPSILON)) {
            const auto axis = n0.get_any_perpendicular();
            return Quat_t<T>(axis.x, axis.y, axis.z, T(0));
        } else {
            Vector3_t<T> c = n0.cross(n1);
            const T s = std::sqrt((T(1) + d) * T(2));
            const T rs = T(1) / s;
            return Quat_t<T>{c.x * rs, c.y * rs, c.z * rs, s / 2};
        }
    }

    [[nodiscard]]
    static constexpr Quat_t<T> from_direction(const Vector3_t<T> & dir){
        // Default direction is the positive Z-axis
        const Vector3_t<T> default_dir(0, 0, 1);
        
        // Normalize the input direction
        const Vector3_t<T> normalized_dir = dir.normalized();
        
        // Calculate the dot product to determine the angle between the vectors
        T dot = default_dir.dot(normalized_dir);
        
        // If the vectors are nearly parallel, return the identity quaternion
        if (std::abs(dot) > T(1) - T(CMP_EPSILON)) {
            return Quat_t<T>::IDENTITY;
        }
        
        // Calculate the rotation axis using the cross product
        Vector3_t<T> axis = default_dir.cross(normalized_dir);
        
        // Calculate the angle between the vectors
        T angle = std::acos(dot);
        
        // Create and return the quaternion representing the rotation
        return Quat_t<T>(axis, angle);
    }

    template<EulerAnglePolicy P = EulerAnglePolicy::XYZ>
    [[nodiscard]]
    static constexpr Quat_t<T> from_euler(const EulerAngle_t<T, P> &euler) {
        Quat_t<T> ret;
        ret.set_euler_xyz({euler.x, euler.y, euler.z});
        return ret;
    }

    [[nodiscard]]
    constexpr Quat_t(const Quat_t &p_q) :
            x(p_q.x),
            y(p_q.y),
            z(p_q.z),
            w(p_q.w) {
    }

    __fast_inline constexpr Quat_t operator=(const Quat_t &p_q) {
        x = p_q.x;
        y = p_q.y;
        z = p_q.z;
        w = p_q.w;
        return *this;
    }

    [[nodiscard]]
    consteval size_t size() const {return 4;}

    [[nodiscard]]
    __fast_inline constexpr T * begin(){return &x;}

    [[nodiscard]]
    __fast_inline constexpr const T * begin() const {return &x;}

    [[nodiscard]]
    __fast_inline constexpr T * end(){return &x + 4;}

    [[nodiscard]]
    __fast_inline constexpr const T * end() const {return &x + 4;}

    [[nodiscard]]
    __fast_inline constexpr  T & operator [](const size_t idx){return (&x)[idx];}

    [[nodiscard]]
    __fast_inline constexpr const T & operator [](const size_t idx) const {return (&x)[idx];}

    [[nodiscard]]
    constexpr T length_squared() const;

    [[nodiscard]]
    constexpr T inv_length() const;

    [[nodiscard]]
    constexpr bool is_equal_approx(const Quat_t & other) const;

    [[nodiscard]]
    constexpr T length() const;
    constexpr void normalize();

    [[nodiscard]]
    constexpr Quat_t normalized() const;

    [[nodiscard]]
    constexpr bool is_normalized() const;

    [[nodiscard]]
    constexpr Quat_t inverse() const;

    [[nodiscard]]
    constexpr T dot(const Quat_t &p_q) const;

    [[nodiscard]]
    constexpr T angle_to(const Quat_t &p_to) const;

    constexpr void set_euler_xyz(const EulerAngle_t<T, EulerAnglePolicy::XYZ> &p_euler);

    [[nodiscard]]
    constexpr Quat_t integral(const Vector3_t<T> & p, const T delta) const {
        const auto k = delta / 2;
        return Quat_t<T>(
            x + k * (-y * p.z + z * p.y + w * p.x),
            y + k * (x * p.z - z * p.x + w * p.y),
            z + k * (-x * p.y + y * p.x + w * p.z),
            w + k * (-x * p.x - y * p.y - z * p.z)
        ).normalized();
    }

    // [[nodiscard]]
    // constexpr Quat_t integral(const Quat_t<T> & q, const T delta) const {

    // }

    [[nodiscard]]
    constexpr Quat_t slerp(const Quat_t &p_to, const T &p_weight) const;

    [[nodiscard]]
    constexpr Quat_t slerpni(const Quat_t &p_to, const T &p_weight) const;

    [[nodiscard]]
    constexpr Quat_t cubic_slerp(const Quat_t &p_b, const Quat_t &p_pre_a, const Quat_t &p_post_b, const T &p_weight) const;

    constexpr void set_axis_angle(const Vector3_t<T> &axis, const T &angle);
    constexpr void get_axis_angle(Vector3_t<T> &r_axis, T &r_angle) const {
        r_angle = 2.0f * acos(w);
        T r = (1.0f) / sqrt(1 - w * w);
        r_axis.x = x * r;
        r_axis.y = y * r;
        r_axis.z = z * r;
    }

    __fast_inline constexpr 
    void operator*=(const Quat_t &p_q);

    [[nodiscard]] __fast_inline constexpr 
    Quat_t operator*(Quat_t && p_q) const;

    [[nodiscard]] __fast_inline constexpr
    Quat_t operator*(const Quat_t & p_q) const;

    // [[nodiscard]] __fast_inline constexpr
    // Quat_t operator*(const Vector3_t<T> &v) const {
    //     return Quat_t(w * v.x + y * v.z - z * v.y,
    //             w * v.y + z * v.x - x * v.z,
    //             w * v.z + x * v.y - y * v.x,
    //             -x * v.x - y * v.y - z * v.z);
    // }

    [[nodiscard]] __fast_inline constexpr
    Vector3_t<T> operator*(const Vector3_t<T> &v) const {
        Vector3_t<T> u(x, y, z);
        Vector3_t<T> uv = u.cross(v);
        return v + ((uv * w) + u.cross(uv)) * 2;
    }

    [[nodiscard]] __fast_inline constexpr
    Quat_t operator*(const T v) const {
        return Quat_t(x * v,  y * v, z * v,  w * v);
    }

    [[nodiscard]] __fast_inline constexpr
    Vector3_t<T> xform(const Vector3_t<T> &v) const {
        Vector3_t<T> u(x, y, z);
        Vector3_t<T> uv = u.cross(v);
        return v + ((uv * w) + u.cross(uv)) * 2;
    }

    [[nodiscard]] __fast_inline constexpr
    Vector3_t<T> xform_up() const {
        // �������� v = (0, 0, 1)
        return Vector3_t<T>(
            2 * (w * y + z * x),      // x ����
            2 * (-w * x + z * y),     // y ����
            2 * (z * z + w * w) - 1   // z ����
        );
    }

    [[nodiscard]] __fast_inline constexpr
    Vector3_t<T> xform_top() const{
        return Vector3_t<T>(
            T(2 * (x * z - w * y)),
            T(2 * (w * x + y * z)),
            T(w * w - x * x - y * y + z * z)
        );
    }

    __fast_inline constexpr
    Quat_t & operator/=(const T &s){return *this = *this / s;};

    [[nodiscard]] __fast_inline constexpr
    Quat_t operator/(const T &s) const;


    // https://blog.csdn.net/xiaoma_bk/article/details/79082629
    template<EulerAnglePolicy P = EulerAnglePolicy::XYZ>
    EulerAngle_t<T, P> to_euler() const {
        auto & q = *this;

        EulerAngle_t<T, P> angles;
    
        // roll (x-axis rotation)

        const auto qx_squ = q.x * q.x;
        const auto qy_squ = q.y * q.y;
        T sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
        T cosr_cosp = 1 - 2 * (qx_squ + qy_squ);

        angles.x = std::atan2(sinr_cosp, cosr_cosp);
    
        // pitch (y-axis rotation)
        T sinp = 2 * (q.w * q.y - q.z * q.x);
        if (std::abs(sinp) >= 1)
            angles.y = sinp > 0 ? T(PI / 2) : T(-PI / 2); // use 90 degrees if out of range
        else
            angles.y = std::asin(sinp);
    
        // yaw (z-axis rotation)
        T siny_cosp = 2 * (q.w * q.z + q.x * q.y);
        T cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
        angles.z = std::atan2(siny_cosp, cosy_cosp);
    
        return angles;
        // return {
        //     .x = std::atan2(sinr_cosp, cosr_cosp);
        //     .y = 2 * (q.w * q.y - q.z * q.x);
        // }
    }
};

template<arithmetic T>
[[nodiscard]] __fast_inline constexpr Quat_t<T> operator*(const arithmetic auto & n, const Quat_t<T> & vec){
    return vec * n;
}

[[nodiscard]] __fast_inline constexpr auto lerp(const Quat_t<arithmetic auto> & a, const Quat_t<arithmetic auto> & b, const arithmetic auto & t){
    return a.slerp(b, t);
}

__fast_inline OutputStream & operator<<(OutputStream & os, const ymd::Quat_t<auto> & value){
    return os << os.brackets<'('>()
        << value.x << os.splitter()
        << value.y << os.splitter()
        << value.z << os.splitter()
        << value.w << os.brackets<')'>();
}

template<arithmetic T>
Quat_t() -> Quat_t<T>;
}


// namespace std{
//     template<typename T>
//     struct tuple_size<ymd::Quat_t<T>> {
//         constexpr static size_t value = 4;
//     };

//     template<size_t N, typename T>
//     struct tuple_element<N, ymd::Quat_t<T>> {
//         using type = T;
//     };

//     template<size_t N, typename T>
//     auto get(ymd::Quat_t<T> & v){
//         static_assert(N < 4);
//         if constexpr (N == 0) return v.x;
//         else if constexpr (N == 1) return v.y;
//         else if constexpr (N == 2) return v.z;
//         else return v.w;
//     }
// }


#include "Quat.tpp"