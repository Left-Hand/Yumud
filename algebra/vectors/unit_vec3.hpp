#pragma once


namespace ymd::math{

template<typename T>
struct Vec3;

template<typename T>
struct [[nodiscard]] UnitVec3 final{
    T x;
    T y;
    T z;

    constexpr T length() const {return 1;}
    constexpr T inv_length() const {return 1;}
    constexpr T length_squared() const {return 1;}

    constexpr UnitVec3<T> normalized() const { return *this; }
    constexpr UnitVec3<T> operator-() const { return UnitVec3<T>{-x, -y, -z}; }
    constexpr UnitVec3<T> operator+() const { return UnitVec3<T>{x, y, z}; }

    template<typename U>
    constexpr Vec3<T> operator *(const U rhs) const { 
        return Vec3<T>{x * rhs, y * rhs, z * rhs};
    }

    template<typename U>
    constexpr Vec3<T> operator /(const U rhs) const { 
        const auto inv_rhs = 1 / rhs;
        return Vec3<T>{x * inv_rhs, y * inv_rhs, z * inv_rhs};
    }

    constexpr T dot(const UnitVec3<T> & other) const {
        return dot(Vec3<T>(*this));
    }

    constexpr T dot(const Vec3<T> & other) const {
        return x * other.x + y * other.y + z * other.z;
    }


    constexpr Vec3<T> cross(const UnitVec3<T> & other) const {
        return cross(Vec3<T>(*this));
    }

    constexpr Vec3<T> cross(const Vec3<T> & other) const {
        return Vec3<T>{y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
    }

    constexpr explicit operator Vec3<T>() const { return Vec3<T>{x, y, z}; }
};
}