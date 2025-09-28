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

#include "core/math/realmath.hpp"
#include "types/vectors/vector2.hpp"
#include "core/math/matrix/static_matrix.hpp"

namespace ymd{
template <arithmetic T>
struct Vec3{
public:
    T x;
    T y;
    T z;

    static constexpr Vec3<T> LEFT       =  Vec3<T>(-1, 0, 0);
    static constexpr Vec3<T> RIGHT      =  Vec3<T>(1, 0, 0);
    static constexpr Vec3<T> UP         =  Vec3<T>(0, 1, 0);
    static constexpr Vec3<T> DOWN       =  Vec3<T>(0, -1, 0);
    static constexpr Vec3<T> FORWARD    =  Vec3<T>(0, 0, 1);
    static constexpr Vec3<T> BACK       =  Vec3<T>(0, 0, -1);

    static constexpr Vec3<T> X_AXIS     =  Vec3<T>(1, 0, 0);
    static constexpr Vec3<T> Y_AXIS     =  Vec3<T>(0, 1, 0);
    static constexpr Vec3<T> Z_AXIS     =  Vec3<T>(0, 0, 1);

    static constexpr Vec3<T> ZERO       =  Vec3<T>(0, 0, 0);
    static constexpr Vec3<T> ONE        =  Vec3<T>(1, 1, 1);

    static constexpr Vec3<T> INF        =  Vec3<T>(INFINITY, INFINITY, INFINITY);

    [[nodiscard]] __fast_inline constexpr Vec3() = delete;

    [[nodiscard]] __fast_inline constexpr Vec3(const Vec3<arithmetic auto>& v) :
        x(static_cast<T>(v.x)), 
        y(static_cast<T>(v.y)), 
        z(static_cast<T>(v.z)) {;}

    [[nodiscard]] __fast_inline constexpr Vec3(
        const Vec2<arithmetic auto>& v, const arithmetic auto z_) : 
        
        x(v.x), y(v.y), z(z_) {;}

    [[nodiscard]] __fast_inline constexpr Vec3(
        const Matrix<auto, 3, 1> mat):
        x(mat(0, 0)), y(mat(1, 0)), z(mat(2, 0)){;}
    

    [[nodiscard]] __fast_inline constexpr Vec3(
        const arithmetic auto _x, 
        const arithmetic auto _y, 
        const arithmetic auto _z
    ): 
        x(static_cast<T>(_x)), 
        y(static_cast<T>(_y)), 
        z(static_cast<T>(_z)){;}

    [[nodiscard]] __fast_inline static constexpr 
    Vec3 from_ones(const arithmetic auto _x){
        return Vec3<T>(_x, _x, _x);}
    [[nodiscard]] __fast_inline static constexpr 
    Vec3 from_rcp(const arithmetic auto _x, const arithmetic auto _y, const arithmetic auto _z){
        return Vec3<T>(static_cast<T>(1)/_x, static_cast<T>(1)/_y, static_cast<T>(1)/_z);}
    [[nodiscard]] __fast_inline static constexpr 
    Vec3 from_rcp(const Vec3<arithmetic auto>& v){
        return Vec3<T>(static_cast<T>(1)/v.x, static_cast<T>(1)/v.y, static_cast<T>(1)/v.z);}

    [[nodiscard]] __fast_inline static constexpr 
    Vec3 from_x00(T _x){
        return Vec3<T>(_x, T(0), T(0));}

    [[nodiscard]] __fast_inline static constexpr 
    Vec3 from_0y0(T _y){
        return Vec3<T>(T(0), _y, T(0));}

    [[nodiscard]] __fast_inline static constexpr 
    Vec3 from_00z(T _z){
        return Vec3<T>(T(0), T(0), _z);}

    template<arithmetic U = T>
    [[nodiscard]] __fast_inline constexpr Vec3(const std::tuple<U, U, U> & v) : 
        x(std::get<0>(v)), 
        y(std::get<1>(v)), 
        z(std::get<2>(v)){;}

    [[nodiscard]] constexpr T & operator [](const size_t idx) { return (&x)[idx];}

    [[nodiscard]] constexpr const T & operator [](const size_t idx) const {return (&x)[idx];}

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr Vec3& operator=(const Vec3<U>& v) {
        x = static_cast<T>(v.x);
        y = static_cast<T>(v.y);
        z = static_cast<T>(v.z);
        return *this;
    };



    template<arithmetic U>
    __fast_inline constexpr 
    Vec3 & operator += (const Vec3<U>& v) {
        x += static_cast<T>(v.x);
        y += static_cast<T>(v.y);
        z += static_cast<T>(v.z);
        return *this;
    }
    
    template<arithmetic U>
    __fast_inline constexpr 
    Vec3 & operator -= (const Vec3<U>& v) {
        x -= static_cast<T>(v.x);
        y -= static_cast<T>(v.y);
        z -= static_cast<T>(v.z);
        return *this;
    }

    template<arithmetic U>
    __fast_inline constexpr 
    Vec3 & operator *= (const Vec3<U>& v) {
        x *= static_cast<T>(v.x);
        y *= static_cast<T>(v.y);
        z *= static_cast<T>(v.z);
        return *this;
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr 
    Vec3<T> increase_x(const U v) const {
        return {x + v, y, z};
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr 
    Vec3<T> increase_y(const U v) const {
        return {x, y + v, z};
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr 
    Vec3<T> increase_z(const U v) const {
        return {x, y, z + v};
    }

    template<arithmetic U>
    __fast_inline constexpr 
    Vec3 & operator *= (const U _v){
        const T v = static_cast<T>(_v);
        x = x * v;
        y = y * v;
        z = z * v;
        return *this;
    }

    template<arithmetic U>
    __fast_inline constexpr 
    Vec3 & operator /= (const Vec3<U>& v) {
        x /= static_cast<T>(v.x);
        y /= static_cast<T>(v.y);
        z /= static_cast<T>(v.z);
        return *this;
    }

    template<arithmetic U>
    __fast_inline constexpr 
    Vec3 & operator /= (const U _v){
        if constexpr(std::is_integral_v<T>){
            const T v = static_cast<T>(_v);
            x /= v;
            y /= v;
            z /= v;
        }else{
            const T inv_v = 1 / static_cast<T>(_v);
            x *= inv_v;
            y *= inv_v;
            z *= inv_v;
        }
        return *this;
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr 
    Vec3 operator *(const U _v) const{
        Vec3 other = *this;
        other *= _v;
        return other;
    }

    [[nodiscard]] __fast_inline constexpr 
    Vec3 operator *(const Vec3<arithmetic auto> & _v) const{
        Vec3 other = *this;
        other *= _v;
        return other;
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr 
    Vec3 operator /(const U _v) const{
        Vec3 other = *this;
        other /= _v;
        return other;
    }

    [[nodiscard]] __fast_inline constexpr 
    Vec3 operator /(const Vec3<arithmetic auto> & _v) const{
        Vec3 other = *this;
        other /= _v;
        return other;
    }

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr 
    Vec3 operator +(const Vec3<U>& other) const {
        Vec3 ret = other;
        return ret += *this;
    }

	[[nodiscard]] __fast_inline constexpr 
    Vec3 minf(arithmetic auto p_scalar) const {
		return Vec3(
            MIN(x, static_cast<T>(p_scalar)), 
            MIN(y, static_cast<T>(p_scalar)), 
            MIN(z, static_cast<T>(p_scalar))
        );
	}

    template<arithmetic U>
    [[nodiscard]] __fast_inline constexpr 
    Vec3 operator -(const Vec3<U> & other) const {
        return Vec3{
            x - static_cast<T>(other.x),
            y - static_cast<T>(other.y),
            z - static_cast<T>(other.z)
        };
    }
    
    [[nodiscard]] __fast_inline constexpr 
    Vec3 operator -() const {
        return Vec3{-x,-y,-z};
    }


    template<arithmetic U>
    [[nodiscard]] constexpr 
    Vec3<T> clampmin(const U _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l < length ? *this * length / l : *this);
    }

    template<arithmetic U>
    [[nodiscard]] constexpr 
    Vec3<T> clampmax(const U _length) const{
        T length = static_cast<T>(_length);
        T l = this->length();
        return (l > length ? *this * length / l : *this);
    }

    [[nodiscard]] constexpr Vec2<T> xy() const{
        return {x,y};
    }


    [[nodiscard]] constexpr __fast_inline 
    T dot(const Vec3<arithmetic auto > &v) const{
        return static_cast<T>(x * v.x + y * v.y + z * v.z);
    }

    [[nodiscard]] constexpr __fast_inline 
    Vec3 max_with(const Vec3<arithmetic auto> &v) const{
        return {
            std::max(x, static_cast<T>(v.x)),
            std::max(y, static_cast<T>(v.y)),
            std::max(z, static_cast<T>(v.z)),
        };
    }

    [[nodiscard]] constexpr __fast_inline 
    Vec3 min_with(const Vec3<arithmetic auto> &v) const{
        return {
            std::min(x, static_cast<T>(v.x)),
            std::min(y, static_cast<T>(v.y)),
            std::min(z, static_cast<T>(v.z)),
        };
    }

    template<arithmetic U>
    __fast_inline constexpr 
    Vec3 cross(const Vec3<U> &u) const{
        return Vec3(
            static_cast<T>(y * u.z - z * u.y),
            static_cast<T>(z * u.x - x * u.z), 
            static_cast<T>(x * u.y - y * u.x)
        );
    }


    [[nodiscard]] __fast_inline constexpr 
    T length() const{
        static_assert(not std::is_integral_v<T>);
        return sqrt(x * x + y * y + z * z);
    }

    [[nodiscard]] __fast_inline constexpr 
    T length_squared() const{
        return x * x + y * y + z * z;
    }

    constexpr void normalize() {
        static_assert(not std::is_integral_v<T>);
        T squ_len = length_squared();
        if (unlikely(squ_len == 0)) {
            x = y = z = 0;
        } else{
            T inv_len = isqrt(squ_len);
            x *= inv_len;
            y *= inv_len;
            z *= inv_len;
        }
    }

    [[nodiscard]] constexpr 
    Vec3 normalized() const {
        static_assert(not std::is_integral_v<T>);
        Vec3 v = *this;
        v.normalize();
        return v;
    }
    [[nodiscard]] Vec3<T> get_any_perpendicular() const {
        // Return the any perpendicular vector by cross product with the Vec3.RIGHT or Vec3.UP,
        // whichever has the greater angle to the current vector with the sign of each element positive.
        // The only essence is "to avoid being parallel to the current vector", and there is no mathematical basis for using Vec3.RIGHT and Vec3.UP,
        // since it could be a different vector depending on the prior branching code Math::abs(x) <= Math::abs(y) && Math::abs(x) <= Math::abs(z).
        // However, it would be reasonable to use any of the axes of the basis, as it is simpler to calculate.
        return cross((std::abs(x) <= std::abs(y) && std::abs(x) <= std::abs(z)) ? 
            Vec3<T>(1, 0, 0) : 
            Vec3<T>(0, 1, 0)).normalized();
    }
    
    [[nodiscard]]
    constexpr bool is_zero() const {
        if constexpr(std::is_integral<T>::value){
            return x == 0 and y == 0 and z == 0;
        }else{
            return is_equal_approx(x, T(0)) 
                and is_equal_approx(y, T(0))
                and is_equal_approx(z, T(0));
        }
    }


    [[nodiscard]]
    static constexpr bool sort_by_x(const Vec3 & a, const Vec3 & b){

        return a.x < b.x;
    };
    [[nodiscard]]
    static constexpr bool sort_by_y(const Vec3 & a, const Vec3 & b){
        return a.y < b.y;
    };

    [[nodiscard]]
    static constexpr bool sort_by_z(const Vec3 & a, const Vec3 & b){
        return a.y < b.y;
    };

    [[nodiscard]]
    static constexpr bool sort_by_length(const Vec3 & a, const Vec3 & b){
        return a.length_squared() < b.length_square();
    };

    template<std::size_t I>
    constexpr auto& get(){
        if constexpr (I == 0) return x;
        else if constexpr (I == 1) return y;
        else if constexpr (I == 2) return z;
    }
    
    template<std::size_t I>
    constexpr const auto& get() const{
        if constexpr (I == 0) return x;
        else if constexpr (I == 1) return y;
        else if constexpr (I == 2) return z;
    }
};

template<arithmetic T>
[[nodiscard]] __fast_inline constexpr Vec3<T> operator*(
    const arithmetic auto & n, 
    const Vec3<T> & vec
){
    return vec * n;
}


[[nodiscard]] __fast_inline constexpr auto lerp(
    const Vec3<arithmetic auto> & a, 
    const Vec3<arithmetic auto> & b, 
    const arithmetic auto & t
){
    return a + (b - a) * t;
}

[[nodiscard]] __fast_inline constexpr auto distance(
    const Vec3<arithmetic auto> & a, 
    const Vec3<arithmetic auto> & b
){
    return (a - b).length();
}

[[nodiscard]] __fast_inline constexpr auto normal(
    const Vec3<arithmetic auto> & from, 
    const Vec3<arithmetic auto> & to
){
    return (to - from).normalized();
}

__fast_inline OutputStream & operator<<(OutputStream & os, const Vec3<auto> & value){
    return os << os.brackets<'('>() << value.x 
        << os.splitter() << value.y 
        << os.splitter() << value.z
        << os.brackets<')'>();
}


template<arithmetic T>
Vec3(T, T, T) -> Vec3<T>;


template<typename T>
struct ToMatrixDispatcher<Vec3<T>>{
    static constexpr auto cast(const Vec3<T>& p){
        return Matrix<T, 3, 1>(p.x, p.y, p.z);
    }
};



using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Vec3i = Vec3<int>;
using Vec3u = Vec3<uint>;


}
