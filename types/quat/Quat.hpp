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
// #include "core/math/fast/conv.hpp"

namespace ymd{

template <arithmetic T>

struct Quat_t{
    T x = {};
    T y = {};
    T z = {};
    T w = {};                

    __fast_inline constexpr Quat_t() :
            x(0),
            y(0),
            z(0),
            w(1) {
    }

    __fast_inline constexpr Quat_t(const auto p_x, const auto p_y, const auto p_z, const auto p_w) :
            x(static_cast<T>(p_x)),
            y(static_cast<T>(p_y)),
            z(static_cast<T>(p_z)),
            w(static_cast<T>(p_w)) {
    }


    Quat_t(const Vector3_t<T> &axis, const T &angle) { set_axis_angle(axis, angle); }



    Quat_t(const Vector3_t<T> &v0, const Vector3_t<T> &v1) // shortest arc
    {
        Vector3_t<T> c = v0.cross(v1);
        T d = v0.dot(v1);

        if (d < T(-1) + T(CMP_EPSILON)) {
            x = 0;
            y = 1;
            z = 0;
            w = 0;
        } else {
            T s = isqrt((T(1) + d) * T(2));
            T rs = T(1) / s;

            x = c.x * rs;
            y = c.y * rs;
            z = c.z * rs;
            w = s / 2;
        }
    }

    Quat_t(const Vector3_t<T> &euler) { set_euler(euler); }
    Quat_t(const Quat_t &p_q) :
            x(p_q.x),
            y(p_q.y),
            z(p_q.z),
            w(p_q.w) {
    }

    __fast_inline constexpr  Quat_t operator=(const Quat_t &p_q) {
        x = p_q.x;
        y = p_q.y;
        z = p_q.z;
        w = p_q.w;
        return *this;
    }

    consteval size_t size() const {return 4;}
    __fast_inline constexpr T * begin(){return &x;}
    __fast_inline constexpr const T * begin() const {return &x;}
    __fast_inline constexpr T * end(){return &x + 4;}
    __fast_inline constexpr const T * end() const {return &x + 4;}
    __fast_inline constexpr  T & operator [](const size_t idx){return (&x)[idx];}
    __fast_inline constexpr const T & operator [](const size_t idx) const {return (&x)[idx];}

    T length_squared() const;
    T inv_length() const;
    bool is_equal_approx(const Quat_t & other) const;
    T length() const;
    void normalize();
    Quat_t normalized() const;
    bool is_normalized() const;
    Quat_t inverse() const;
    T dot(const Quat_t &p_q) const;
    T angle_to(const Quat_t &p_to) const;

    __no_inline void set_euler_xyz(const Vector3_t<T> &p_euler);
    __no_inline void set_euler_yxz(const Vector3_t<T> &p_euler);
    void set_euler(const Vector3_t<T> &p_euler) { set_euler_yxz(p_euler); };

    Quat_t integral(const Vector3_t<T> & p) const {
        return Quat_t<T>(
            x + real_t(0.5f) * (-y * p.z + z * p.y + w * p.x),
            y + real_t(0.5f) * (x * p.z - z * p.x + w * p.y),
            z + real_t(0.5f) * (-x * p.y + y * p.x + w * p.z),
            w + real_t(0.5f) * (-x * p.x - y * p.y - z * p.z)
        ).normalized();
    }

    Quat_t slerp(const Quat_t &p_to, const T &p_weight) const;
    Quat_t slerpni(const Quat_t &p_to, const T &p_weight) const;
    Quat_t cubic_slerp(const Quat_t &p_b, const Quat_t &p_pre_a, const Quat_t &p_post_b, const T &p_weight) const;

    void set_axis_angle(const Vector3_t<T> &axis, const T &angle);
    void get_axis_angle(Vector3_t<T> &r_axis, T &r_angle) const {
        r_angle = 2.0f * acos(w);
        T r = (1.0f) / sqrt(1 - w * w);
        r_axis.x = x * r;
        r_axis.y = y * r;
        r_axis.z = z * r;
    }

    __fast_inline void operator*=(const Quat_t &p_q);

    __fast_inline constexpr
    Quat_t operator*(Quat_t && p_q) const;

    __fast_inline constexpr
    Quat_t operator*(const Quat_t & p_q) const;

    Quat_t operator*(const Vector3_t<T> &v) const {
        return Quat_t(w * v.x + y * v.z - z * v.y,
                w * v.y + z * v.x - x * v.z,
                w * v.z + x * v.y - y * v.x,
                -x * v.x - y * v.y - z * v.z);
    }

    Quat_t operator*(const T v) const {
        return Quat_t(v * x,  v * y,  v * z,  w * v);
    }

    __fast_inline constexpr 
    Vector3_t<T> xform(const Vector3_t<T> &v) const {
        Vector3_t<T> u(x, y, z);
        Vector3_t<T> uv = u.cross(v);
        return v + ((uv * w) + u.cross(uv)) * 2;
    }

    __fast_inline constexpr 
    Vector3_t<T> xform_up() const {
        // 假设输入 v = (0, 0, 1)
        return Vector3_t<T>(
            2 * (w * y + z * x),      // x 分量
            2 * (-w * x + z * y),     // y 分量
            2 * (z * z + w * w) - 1   // z 分量
        );
    }

    __fast_inline constexpr 
    auto xform_top() const{
        return Vector3_t<T>(
            T(2 * (x * z - w * y)),
            T(2 * (w * x + y * z)),
            T(w * w - x * x - y * y + z * z)
        );
    }


    void operator/=(const T &s){*this = *this / s;};

    Quat_t operator/(const T &s) const;




};

template<arithmetic T>
__fast_inline constexpr Quat_t<T> operator*(const arithmetic auto & n, const Quat_t<T> & vec){
    return vec * n;
}

__fast_inline constexpr auto lerp(const Quat_t<arithmetic auto> & a, const Quat_t<arithmetic auto> & b, const arithmetic auto & t){
    return a.slerp(b, t);
}

__fast_inline OutputStream & operator<<(OutputStream & os, const ymd::Quat_t<auto> & value){
    const auto splt = os.splitter();
    return os << os.brackets<'('>() << 
        value.x << splt << 
        value.y << splt << 
        value.z << splt << 
        value.w << os.brackets<')'>();
}

template<arithmetic T>
Quat_t() -> Quat_t<T>;
}



#include "Quat.tpp"