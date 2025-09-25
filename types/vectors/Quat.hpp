#pragma once


#include "types/vectors/vector3.hpp"
#include "types/transforms/euler.hpp"

namespace ymd{

template <arithmetic T>

struct Quat{
    T x;
    T y;
    T z;
    T w;

    static_assert(not std::is_integral_v<T>);

    static constexpr Quat<T> IDENTITY = Quat<T>::from_xyzw(
        static_cast<T>(0),
        static_cast<T>(0),
        static_cast<T>(0),
        static_cast<T>(1)
    );

    [[nodiscard]]
    __fast_inline static constexpr Quat from_xyzw(
        const T p_x, const T p_y, const T p_z, const T p_w){
        return Quat<T> {
            .x = p_x,
            .y = p_y,
            .z = p_z,
            .w = p_w
        };
    }

    [[nodiscard]] 
    __fast_inline static constexpr Quat from_array(
        std::array<T, 4> p_array
    ){
        return from_xyzw(
            p_array[0], 
            p_array[1], 
            p_array[2], 
            p_array[3]
        );
    }

    [[nodiscard]]
    __fast_inline static constexpr Quat from_axis_angle(const Vec3<T> &axis, const Angle<T> angle) {
        Quat ret;
        ret.set_axis_angle(axis, angle);
        return ret;
    }

    [[nodiscard]]
    static constexpr Quat from_shortest_arc(
        const Vec3<T> &v0, 
        const Vec3<T> &v1
    ){
        const Vec3<T> n0 = v0.normalized();
        const Vec3<T> n1 = v1.normalized();

        T d = v0.dot(v1);

        if (std::abs(d) > T(1) - T(CMP_EPSILON)) {
            const auto axis = n0.get_any_perpendicular();
            return from_xyzw(axis.x, axis.y, axis.z, T(0));
        } else {
            Vec3<T> c = n0.cross(n1);
            const T s = std::sqrt((T(1) + d) * T(2));
            const T inv_s = T(1) / s;
            return from_xyzw(c.x * inv_s, c.y * inv_s, c.z * inv_s, s / 2);
        }
    }

    [[nodiscard]]
    static constexpr Quat<T> from_direction(const Vec3<T> & dir){
        // Default direction is the positive Z-axis
        const Vec3<T> default_dir(0, 0, 1);
        
        // Normalize the input direction
        const Vec3<T> normalized_dir = dir.normalized();
        
        // Calculate the dot product to determine the angle between the vectors
        T dot_product = default_dir.dot(normalized_dir);
        
        // If the vectors are nearly parallel, return the identity quaternion
        if (std::abs(dot_product) > T(1) - T(CMP_EPSILON)) {
            return IDENTITY;
        }
        
        // Calculate the rotation axis using the cross product
        Vec3<T> axis = default_dir.cross(normalized_dir);
        
        // Calculate the angle between the vectors
        const auto angle = Angle<T>::from_radians(std::acos(dot_product));
        
        // Create and return the quaternion representing the rotation
        return from_axis_angle(axis, angle);
    }


    template<EulerAnglePolicy P = EulerAnglePolicy::XYZ>
    [[nodiscard]]
    static constexpr Quat<T> from_euler_angles(const EulerAngle<T, P> &euler_angle) {
        Quat<T> ret;
        ret.set_euler_angles(euler_angle.x, euler_angle.y, euler_angle.z);
        return ret;
    }

    [[nodiscard]] 
    constexpr bool is_pure_real() const {
        return w == 1 && x == 0 && y == 0 && z == 0;
    }

    [[nodiscard]]
    constexpr bool is_pure_imaginary() const {
        return w == 0 && x != 0 && y != 0 && z != 0;
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
    __fast_inline constexpr const T  operator [](const size_t idx) const {return (&x)[idx];}

    [[nodiscard]]
    constexpr T angle_to(const Quat<T> &p_to) const {
        T d = dot(p_to);
        return acosf(CLAMP(d * d * 2 - 1, -1, 1));
    }

    [[nodiscard]]
    constexpr T dot(const Quat<T> &p_q) const {
        return T(x * p_q.x + y * p_q.y + z * p_q.z + w * p_q.w);
    }

    [[nodiscard]]
    constexpr T length_squared() const{
        return dot(*this);
    }

    [[nodiscard]]
    constexpr T inv_length() const {
        return T(isqrt(x * x + y * y + z * z + w * w));
    }

    constexpr void operator*=(const Quat<T> &p_q) {
        set(
            static_cast<T>(w * p_q.x + x * p_q.w + y * p_q.z - z * p_q.y),
            static_cast<T>(w * p_q.y + y * p_q.w + z * p_q.x - x * p_q.z),
            static_cast<T>(w * p_q.z + z * p_q.w + x * p_q.y - y * p_q.x),
            static_cast<T>(w * p_q.w - x * p_q.x - y * p_q.y - z * p_q.z)
        );
    }

    [[nodiscard]]
    constexpr Quat<T> operator*(const Quat<T> & p_q) const {
        Quat<T> r = *this;
        r *= p_q;
        return r;
    }

    [[nodiscard]]
    constexpr Quat<T> operator*(Quat<T> && p_q) const {
        p_q *= *this;
        return p_q;
    }

    [[nodiscard]]
    constexpr bool is_equal_approx(const Quat<T> & other) const {
        return is_equal_approx(x, other.x) && is_equal_approx(y, other.y) && is_equal_approx(z, other.z) && is_equal_approx(w, other.w);
    }

    [[nodiscard]]
    constexpr T length() const {
        return sqrt(length_squared());
    }

    [[nodiscard]]
    constexpr Quat<T> normalized() const {
        const auto ilen = inv_length();
        const auto ret = Quat<T>(x * ilen, y * ilen, z * ilen, w * ilen);
        return ret;
    }

    [[nodiscard]]
    constexpr bool is_normalized() const {
        return is_equal_approx(length_squared(), T(1)); //use less epsilon
    }

    [[nodiscard]]
    constexpr Quat<T> inverse() const {
        return from_xyzw(-x, -y, -z, w);
    }


    [[nodiscard]]
    constexpr Quat<T> slerp(const Quat<T> &p_to, const T p_weight) const {
        // Quat<T> to1 = ZERO;
        struct {
            T x;
            T y;
            T z;
            T w;
        } to1;

        T omega = 0;
        T sinom = 0;
        T scale0 = 0;
        T scale1 = 0;

        // calc cosine
        T cosom = dot(p_to);

        // adjust signs (if necessary)
        if (cosom < T(0)) {
            cosom = -cosom;
            to1.x = -p_to.x;
            to1.y = -p_to.y;
            to1.z = -p_to.z;
            to1.w = -p_to.w;
        } else {
            to1.x = p_to.x;
            to1.y = p_to.y;
            to1.z = p_to.z;
            to1.w = p_to.w;
        }

        // calculate coefficients

        if ((T(1) - cosom) > T(CMP_EPSILON)) {
            // standard case (slerp)
            omega = std::acos(cosom);
            sinom = std::sinf(omega);
            const auto inv_sinom = 1 / sinom;
            scale0 = std::sinf((1 - p_weight) * omega) * inv_sinom;
            scale1 = std::sinf(p_weight * omega) * inv_sinom;
        } else {
            // "from" and "to" Quat<T>s are very close
            //  ... so we can do a linear interpolation
            scale0 = T(1) - p_weight;
            scale1 = p_weight;
        }
        // calculate final values
        return from_xyzw(
                scale0 * x + scale1 * to1.x,
                scale0 * y + scale1 * to1.y,
                scale0 * z + scale1 * to1.z,
                scale0 * w + scale1 * to1.w);
    }

    [[nodiscard]]
    constexpr Quat<T> slerpni(const Quat<T> &p_to, const T p_weight) const {
        const Quat<T> &from = *this;

        T dot = from.dot(p_to);

        if (abs(dot) > T(0.9999)) {
            return from;
        }

        T theta = acos(dot),
                sinT = 1 / sinf(theta),
                newFactor = sinf(p_weight * theta) * sinT,
                invFactor = sinf((1 - p_weight) * theta) * sinT;

        return Quat<T>(invFactor * from.x + newFactor * p_to.x,
                invFactor * from.y + newFactor * p_to.y,
                invFactor * from.z + newFactor * p_to.z,
                invFactor * from.w + newFactor * p_to.w);
    }

    [[nodiscard]]
    constexpr Quat<T> cubic_slerp(const Quat<T> &p_b, const Quat<T> &p_pre_a, const Quat<T> &p_post_b, const T p_weight) const {

        T t2 = (T(1) - p_weight) * p_weight * 2;
        Quat<T> sp = this->slerp(p_b, p_weight);
        Quat<T> sq = p_pre_a.slerpni(p_post_b, p_weight);
        return sp.slerpni(sq, t2);
    }

    // set_euler_angles expects a vector containing the Euler angles in the format
    // (ax,ay,az), where ax is the angle of rotation around x axis,
    // and similar for other axes.
    // This implementation uses XYZ convention (Z is the first rotation).
    constexpr void set_euler_angles(
        const Angle<T> euler_x, 
        const Angle<T> euler_y, 
        const Angle<T> euler_z
    ) {
        // R = X(a1).Y(a2).Z(a3) convention for Euler angles.
        // Conversion to Quat<T> as listed in https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770024290.pdf (page A-2)
        // a3 is the angle of the first rotation, following the notation in this reference.

        auto [sin_a1, cos_a1] = (euler_x / 2).sincos();
        auto [sin_a2, cos_a2] = (euler_y / 2).sincos();
        auto [sin_a3, cos_a3] = (euler_z / 2).sincos();

        set(
            +sin_a1 * cos_a2 * cos_a3 + sin_a2 * sin_a3 * cos_a1,
            -sin_a1 * sin_a3 * cos_a2 + sin_a2 * cos_a1 * cos_a3,
            +sin_a1 * sin_a2 * cos_a3 + sin_a3 * cos_a1 * cos_a2,
            -sin_a1 * sin_a2 * sin_a3 + cos_a1 * cos_a2 * cos_a3
        );
    }

    [[nodiscard]]
    constexpr Quat integral(const Vec3<T> & norm_dir, const T delta) const {
        const auto k = delta / 2;
        return from_xyzw(
            x + k * (-y * norm_dir.z + z * norm_dir.y + w * norm_dir.x),
            y + k * ( x * norm_dir.z - z * norm_dir.x + w * norm_dir.y),
            z + k * (-x * norm_dir.y + y * norm_dir.x + w * norm_dir.z),
            w + k * (-x * norm_dir.x - y * norm_dir.y - z * norm_dir.z)
        ).normalized();
    }


    constexpr void set_axis_angle(const Vec3<T> &axis, const Angle<T> angle){
        T d = axis.length();
        if (d == 0) {
            set(T(0), T(0), T(0), T(0));
        } else {
            const auto half_angle = angle * static_cast<T>(0.5);
            const auto [sin_angle, cos_angle] = half_angle.sincos();
            const T s = sin_angle / d;
            set(
                axis.x * s, 
                axis.y * s, 
                axis.z * s,
                cos_angle
            );
        }
    }


    [[nodiscard]] __fast_inline constexpr
    Vec3<T> operator*(const Vec3<T> &v) const {
        Vec3<T> u(x, y, z);
        Vec3<T> uv = u.cross(v);
        return v + ((uv * w) + u.cross(uv)) * 2;
    }

    [[nodiscard]] __fast_inline constexpr
    Vec3<T> xform(const Vec3<T> &v) const {
        Vec3<T> u(x, y, z);
        Vec3<T> uv = u.cross(v);
        return v + ((uv * w) + u.cross(uv)) * 2;
    }

    [[nodiscard]] __fast_inline constexpr
    Vec3<T> xform_up() const {
        return Vec3<T>(
            2 * (w * y + z * x),
            2 * (-w * x + z * y),
            2 * (z * z + w * w) - 1
        );
    }

    [[nodiscard]] __fast_inline constexpr
    Vec3<T> xform_top() const{
        return Vec3<T>(
            T(2 * (x * z - w * y)),
            T(2 * (w * x + y * z)),
            T(w * w - x * x - y * y + z * z)
        );
    }

    // https://blog.csdn.net/xiaoma_bk/article/details/79082629
    template<EulerAnglePolicy P = EulerAnglePolicy::XYZ>
    [[nodiscard]] constexpr EulerAngle<T, P> to_euler() const {
        auto & q = *this;

        EulerAngle<T, P> angles;
    
        // roll (x-axis rotation)

        const auto qx_squ = square(q.x);
        const auto qy_squ = square(q.y);
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
    }

    Quat<T> conj() const{
        return Quat<T>(-x, -y, -z, w);
    }

private:
    void set(T _x, T _y, T _z, T _w){
        this->x = _x;
        this->y = _y;
        this->z = _z;
        this->w = _w;
    }
};


[[nodiscard]] __fast_inline constexpr auto lerp(
    const Quat<arithmetic auto> & a, 
    const Quat<arithmetic auto> & b, 
    const arithmetic auto t
){
    return a.slerp(b, t);
}

__fast_inline OutputStream & operator<<(OutputStream & os, const ymd::Quat<auto> & value){
    return os << os.brackets<'('>()
        << value.x << os.splitter()
        << value.y << os.splitter()
        << value.z << os.splitter()
        << value.w << os.brackets<')'>();
}

template<arithmetic T>
Quat() -> Quat<T>;
}
