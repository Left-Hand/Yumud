#ifndef __QUAT_HPP__
#define __QUAT_HPP__

#include "vector3/vector3_t.hpp"

template <arithmetic T>

class Quat_t{
    public:
        T x = T(0);
        T y = T(0);
        T z = T(0);
        T w = T(0);

        inline Quat_t() :
                x(0),
                y(0),
                z(0),
                w(1) {
        }
    
        inline Quat_t(T p_x, T p_y, T p_z, T p_w) :
                x(p_x),
                y(p_y),
                z(p_z),
                w(p_w) {
        }


        T length_squared() const;
        bool is_equal_approx(const Quat_t & other) const;
        T length() const;
        void normalize();
        Quat_t normalized() const;
        bool is_normalized() const;
        Quat_t inverse() const;
        T dot(const Quat_t &p_q) const;
        T angle_to(const Quat_t &p_to) const;

        void set_euler_xyz(const Vector3 &p_euler);
        // Vector3 get_euler_xyz() const;
        void set_euler_yxz(const Vector3 &p_euler);
        // Vector3 get_euler_yxz() const;

        void set_euler(const Vector3 &p_euler) { set_euler_yxz(p_euler); };
        // Vector3 get_euler() const { return get_euler_yxz(); };

        Quat_t slerp(const Quat_t &p_to, const T &p_weight) const;
        Quat_t slerpni(const Quat_t &p_to, const T &p_weight) const;
        Quat_t cubic_slerp(const Quat_t &p_b, const Quat_t &p_pre_a, const Quat_t &p_post_b, const T &p_weight) const;

        void set_axis_angle(const Vector3 &axis, const T &angle);
        void get_axis_angle(Vector3 &r_axis, T &r_angle) const {
            r_angle = 2.0f * acos(w);
            T r = (1.0f) / sqrt(1 - w * w);
            r_axis.x = x * r;
            r_axis.y = y * r;
            r_axis.z = z * r;
        }

        void operator*=(const Quat_t &p_q);
        Quat_t operator*(const Quat_t &p_q) const;

        Quat_t operator*(const Vector3 &v) const {
            return Quat_t(w * v.x + y * v.z - z * v.y,
                    w * v.y + z * v.x - x * v.z,
                    w * v.z + x * v.y - y * v.x,
                    -x * v.x - y * v.y - z * v.z);
        }

        Vector3 xform(const Vector3 &v) const {
            Vector3 u(x, y, z);
            Vector3 uv = u.cross(v);
            return v + ((uv * w) + u.cross(uv)) * ((T)2);
        }


        void operator/=(const T &s){*this = *this / s;};

        Quat_t operator/(const T &s) const;

        inline void set(T p_x, T p_y, T p_z, T p_w) {
            x = p_x;
            y = p_y;
            z = p_z;
            w = p_w;
        }


        Quat_t(const Vector3_t<auto> &axis, const T &angle) { set_axis_angle(axis, angle); }

        Quat_t(const Vector3_t<auto> &euler) { set_euler(euler); }
        Quat_t(const Quat_t &p_q) :
                x(p_q.x),
                y(p_q.y),
                z(p_q.z),
                w(p_q.w) {
        }

        Quat_t operator=(const Quat_t &p_q) {
            x = p_q.x;
            y = p_q.y;
            z = p_q.z;
            w = p_q.w;
            return *this;
        }

        Quat_t(const Vector3 &v0, const Vector3 &v1) // shortest arc
        {
            Vector3 c = v0.cross(v1);
            T d = v0.dot(v1);

            if (d < real_t(-1) + real_t(CMP_EPSILON)) {
                x = 0;
                y = 1;
                z = 0;
                w = 0;
            } else {
                T s = sqrt((T(1) + d) * T(2));
                T rs = T(1) / s;

                x = c.x * rs;
                y = c.y * rs;
                z = c.z * rs;
                w = s * real_t(0.5);
            }
        }
};

using Quat = Quat_t<real_t>;
using Quatf = Quat_t<float>;


#include "Quat_t.tpp"

#endif