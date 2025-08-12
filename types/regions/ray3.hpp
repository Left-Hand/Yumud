#pragma once

#include "types/vectors/vector3.hpp"
#include "core/utils/Option.hpp"

namespace ymd{

template<typename T>
struct Ray3{
    Vec3<T> base;
    Vec3<T> direction;

    [[nodiscard]] constexpr Ray3(){;}

    [[nodiscard]] constexpr Ray3(const Ray3<T> & other) = default;
    [[nodiscard]] static constexpr Ray3 
    from_base_and_dir(
        const Vec3<T> & _base, 
        const Vec3<T> & _direction
    ){
        return Ray3{_base, _direction.normalized()};
    }

    [[nodiscard]] static constexpr Ray3 
    from_start_and_stop(
        const Vec3<T> & _start, 
        const Vec3<T> & _stop
    ){
        return Ray3{_start, (_stop - _start).normalized()};
    }

    [[nodiscard]] constexpr Vec3<T>
    point_at_length(const T length) const {
        return base + direction * length;
    }

    [[nodiscard]] constexpr Ray3<T>
    conj() const {
        return Ray3{base, -direction};
    }

    //构造经过某点的法平面 计算到点的距离
    [[nodiscard]] constexpr T distance_to_point(const Vec3<T>& point) const {
        Vec3<T> diff = point - base;
        return (diff - direction * diff.dot(direction)).length();
    }

    //构造经过某点的法平面 计算与法平面的交点
    [[nodiscard]] constexpr Vec3<T> project_point(const Vec3<T>& point) const {
        T t = (point - base).dot(direction);
        return base + direction * t;
    }

    //计算经过方向法向量反射后的射线
    [[nodiscard]] constexpr Ray3<T> reflect(const Vec3<T> & normal) const {
        const auto unit_normal = normal.normalize();
        return Ray3<T>::from_base_and_dir(
            base, 
            direction - unit_normal * (2 * direction.dot(unit_normal))
        );
    }


    //计算与球体的交点，返回包含两个交点距离(t0, t1)的Option，无交点时返回None
    [[nodiscard]] constexpr Option<std::pair<T, T>> 
    intersect_sphere(const Vec3<T>& center, T radius) const {
        const Vec3<T> oc = base - center;
        const T a = direction.dot(direction);
        const T b = 2 * oc.dot(direction);
        const T c = oc.dot(oc) - radius * radius;
        const T discriminant = b * b - 4 * a * c;
        if (discriminant < 0) return None;
        // Return near and far intersection distances
        const T sqrt_discriminant = std::sqrt(discriminant);
        const T inv_2a = T(1) / (2 * a);
        T t0 = (-b - sqrt_discriminant) * inv_2a;
        T t1 = (-b + sqrt_discriminant) * inv_2a;
        return Some(std::make_pair(t0, t1));
    }
private:
    [[nodiscard]] constexpr Ray3(
        const Vec3<T> & _base, 
        const Vec3<T> & _direction
    ):
        base(_base),
        direction(_direction){;}
};

template<typename T>
Ray3() -> Ray3<T>;

__fast_inline OutputStream & operator<<(OutputStream & os, const Ray3<auto> & value){
    return os << os.brackets<'('>() << 
        value.base << os.splitter() << 
        value.direction << os.brackets<')'>();
}

}