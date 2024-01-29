// #ifndef __QUAT_HPP__
// #define __QUAT_HPP__

// #include <math.h>
// #include "../Vector3/Vector3.hpp"

// class Quat{
//     public:
//         float x = 0.0f;
//         float y = 0.0f;
//         float z = 0.0f;
//         float w = 0.0f;

//         float length_squared() const;
//         bool is_equal_approx(const Quat &p_quat) const;
//         float length() const;
//         void normalize();
//         Quat normalized() const;
//         bool is_normalized() const;
//         Quat inverse() const;
//         float dot(const Quat &p_q) const;
//         float angle_to(const Quat &p_to) const;

//         void set_euler_xyz(const Vector3 &p_euler);
//         // Vector3 get_euler_xyz() const;
//         void set_euler_yxz(const Vector3 &p_euler);
//         // Vector3 get_euler_yxz() const;

//         void set_euler(const Vector3 &p_euler) { set_euler_yxz(p_euler); };
//         // Vector3 get_euler() const { return get_euler_yxz(); };

//         Quat slerp(const Quat &p_to, const float &p_weight) const;
//         Quat slerpni(const Quat &p_to, const float &p_weight) const;
//         Quat cubic_slerp(const Quat &p_b, const Quat &p_pre_a, const Quat &p_post_b, const float &p_weight) const;

//         void set_axis_angle(const Vector3 &axis, const float &angle);
//         void get_axis_angle(Vector3 &r_axis, float &r_angle) const {
//             r_angle = 2.0f * acosf(w);
//             float r = (1.0f) / sqrtf(1 - w * w);
//             r_axis.x = x * r;
//             r_axis.y = y * r;
//             r_axis.z = z * r;
//         }

//         void operator*=(const Quat &p_q);
//         Quat operator*(const Quat &p_q) const;

//         Quat operator*(const Vector3 &v) const {
//             return Quat(w * v.x + y * v.z - z * v.y,
//                     w * v.y + z * v.x - x * v.z,
//                     w * v.z + x * v.y - y * v.x,
//                     -x * v.x - y * v.y - z * v.z);
//         }

//         Vector3 xform(const Vector3 &v) const {
//             Vector3 u(x, y, z);
//             Vector3 uv = u.cross(v);
//             return v + ((uv * w) + u.cross(uv)) * ((float)2);
//         }

//         // void operator+=(const Quat &p_q);
//         // void operator-=(const Quat &p_q);
//         // void operator*=(const float &s);
//         void operator/=(const float &s){*this = *this / s;};
//         // Quat operator+(const Quat &q2) const;
//         // Quat operator-(const Quat &q2) const;
//         // Quat operator-() const;
//         // Quat operator*(const float &s) const;
//         Quat operator/(const float &s) const;

//         // bool operator==(const Quat &p_quat) const;
//         // bool operator!=(const Quat &p_quat) const;

//         inline void set(float p_x, float p_y, float p_z, float p_w) {
//             x = p_x;
//             y = p_y;
//             z = p_z;
//             w = p_w;
//         }

//         inline Quat(float p_x, float p_y, float p_z, float p_w) :
//                 x(p_x),
//                 y(p_y),
//                 z(p_z),
//                 w(p_w) {
//         }

//         Quat(const Vector3 &axis, const float &angle) { set_axis_angle(axis, angle); }

//         Quat(const Vector3 &euler) { set_euler(euler); }
//         Quat(const Quat &p_q) :
//                 x(p_q.x),
//                 y(p_q.y),
//                 z(p_q.z),
//                 w(p_q.w) {
//         }

//         Quat operator=(const Quat &p_q) {
//             x = p_q.x;
//             y = p_q.y;
//             z = p_q.z;
//             w = p_q.w;
//             return *this;
//         }

//         Quat(const Vector3 &v0, const Vector3 &v1) // shortest arc
//         {
//             Vector3 c = v0.cross(v1);
//             float d = v0.dot(v1);

//             if (d < -1.0 + CMP_EPSILON) {
//                 x = 0;
//                 y = 1;
//                 z = 0;
//                 w = 0;
//             } else {
//                 float s = sqrtf((1.0 + d) * 2.0);
//                 float rs = 1.0 / s;

//                 x = c.x * rs;
//                 y = c.y * rs;
//                 z = c.z * rs;
//                 w = s * 0.5;
//             }
//         }

//         inline Quat() :
//                 x(0),
//                 y(0),
//                 z(0),
//                 w(1) {
//         }
// };

// #endif