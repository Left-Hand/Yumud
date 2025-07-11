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

namespace ymd{


template<arithmetic T>
constexpr Vector2<T> Vector2<T>::abs() const{
    return Vector2<T>(fabs(x), fabs(y));
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::ceil() const{

    return Vector2<T>(ceilf(x), ceilf(y));
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::floor() const{

    return Vector2<T>(floorf(x), floorf(y));
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::round() const{

    return Vector2<T>(roundf(x), roundf(y));
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::clamp(const arithmetic auto & _min, const arithmetic auto & _max) const {
    T min = static_cast<T>(_min);
    T max = static_cast<T>(_max);
    T l = this->length();
    if (l > max){
        return *this * max / l;
    }
    else if(l < min){
        return *this * min / l;
    }
    return *this;
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::dir_to(const Vector2<T> & b) const{
    return (b - *this).normalized();
}

template<arithmetic T>
constexpr T Vector2<T>::dist_to(const Vector2<T> & b) const{
    return (b - *this).length();
}

template<arithmetic T>
constexpr T Vector2<T>::dist_squared_to(const Vector2<T> & b) const{
    return (b - *this).length_squared();
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::reflect(const Vector2<T> & n) const {
    return 2 * n * this->dot(n) - *this;
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::bounce(const Vector2<T> & n) const {
    return -reflect(n);
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::lerp(const Vector2<T> & b, const T _t) const{
    static_assert(not std::is_integral_v<T>);
    return *this * (1-_t)+b * _t;
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::slerp(const Vector2<T> & b, const T _t) const{
    static_assert(not std::is_integral_v<T>);
    return lerp(b, sin(static_cast<T>(PI / 2) * _t));
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::posmod(const arithmetic auto & mod) const{
    return Vector2<T>( fmod(x, mod), fmod(y, mod));
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::posmodv(const Vector2<T> & b) const{
    return Vector2<T>(fmod(x, b.x), fmod(y, b.y));
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::project(const Vector2<T> & b) const{
    return (this->dot(b)) * b / b.length_squared();
}

template<arithmetic T>
constexpr T Vector2<T>::project(const T & rad) const{
    auto [s,c] = sincos(rad);
    return (this->x) * c + (this->y) * s;

}

template<arithmetic T>
constexpr bool Vector2<T>::is_equal_approx(const Vector2<T> & b) const{
    return ymd::is_equal_approx(x, b.x) && ymd::is_equal_approx(y, b.y);
}

template<arithmetic T>
constexpr bool Vector2<T>::has_point(const Vector2<auto> & _v) const{
    bool ret = true;
    Vector2<T> v = _v;

    if(x < 0) ret &= (x <= v.x && v.x <= 0);
    else ret &= (0 <= v.x && v.x <= x);
    if(y < 0) ret &= (y <= v.y && v.y <= 0);
    else ret &= (0 <= v.y && v.y <= y);

    return ret;
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::move_toward(const Vector2<T> & b, const T delta) const{
    if (!is_equal_approx(b)){
        Vector2<T> d = b - *this;
        return *this + d.clampmax(delta);
    }
    return *this;
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::slide(const Vector2<T> & n) const {
    return *this - n * this->dot(n);
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::sign() const{
    return Vector2<T>(sign(x), sign(y));
}

template<arithmetic T>
constexpr Vector2<T> Vector2<T>::snapped(const Vector2<T> &by) const{
    return Vector2<T>(snap(x, by.x), snap(y, by.y));
}

template<arithmetic T>
constexpr __fast_inline Vector2<T> Vector2<T>::normalized() const{
    static_assert(not std::is_integral_v<T>);
    return (*this) * isqrt(this->length_squared());
}

template<arithmetic T>
constexpr __fast_inline T Vector2<T>::dot(const Vector2<T> & with) const{
    return (x*with.x + y*with.y);
}

template<arithmetic T>
constexpr __fast_inline T Vector2<T>::cross(const Vector2<T> & with) const{
    return (x*with.y - y*with.x);
}


template<arithmetic T>
constexpr __fast_inline Vector2<T> Vector2<T>::improduct(const Vector2<T> & b) const{
    return Vector2<T>(x*b.x - y*b.y, x*b.y + y*b.x);
}

template<arithmetic T>
constexpr __fast_inline Vector2<T> Vector2<T>::rotated(const T r) const{
    static_assert(not std::is_integral_v<T>);
    auto [s, c] = sincos(r);
    return this->improduct(Vector2<T>(c, s));
}

#define VECTOR2_COMPARE_IM_OPERATOR(op) \
\
template <arithmetic T, arithmetic U> \
constexpr __fast_inline bool operator op (const Vector2<T>& lhs, const U& rhs) { \
    T absrhs = static_cast<T>(abs(rhs)); \
    return lhs.length_squared() op (absrhs * absrhs); \
} \
\
template <arithmetic T, arithmetic U> \
constexpr __fast_inline bool operator op (const U& lhs, const Vector2<T>& rhs) { \
    T abslhs = static_cast<T>(abs(lhs)); \
    return (abslhs * abslhs) op rhs.length_squared(); \
}\
template <arithmetic T, arithmetic U> \
constexpr __fast_inline bool operator op (const Vector2<T>& lhs, const Vector2<U>& rhs) { \
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);\
}\


VECTOR2_COMPARE_IM_OPERATOR(<)
VECTOR2_COMPARE_IM_OPERATOR(<=)
VECTOR2_COMPARE_IM_OPERATOR(>)
VECTOR2_COMPARE_IM_OPERATOR(>=)
VECTOR2_COMPARE_IM_OPERATOR(==)
VECTOR2_COMPARE_IM_OPERATOR(!=)

template <arithmetic T, arithmetic U = T>
constexpr __fast_inline Vector2<T> operator +(const Vector2<T> &p_vector2, const Vector2<U> &d_vector2){
    Vector2<T> ret = p_vector2;
    ret += d_vector2;
    return ret;
}

template <arithmetic T, arithmetic U = T>
constexpr __fast_inline Vector2<T> operator -(const Vector2<T> &p_vector2, const Vector2<U> &d_vector2){
    Vector2<T> ret = p_vector2;
    ret -= d_vector2;
    return ret;
}

template <arithmetic T, arithmetic U = T>
constexpr __fast_inline Vector2<T> operator *(const arithmetic auto &lvalue, const Vector2<U> &p_vector2){
    Vector2<T> ret = p_vector2;
    ret *= lvalue;
    return ret;
}


template <arithmetic T,arithmetic U = T >
constexpr Vector2<T> operator/(const Vector2<T> &p_vector2, const Vector2<U> &d_vector2){
    Vector2<T> final = p_vector2;
    final /= d_vector2;
    return final;
}

template<arithmetic T>
Vector2() -> Vector2<T>;

}