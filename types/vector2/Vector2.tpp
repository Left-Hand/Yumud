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
constexpr Vector2_t<T> Vector2_t<T>::abs() const{
    return Vector2_t<T>(fabs(x), fabs(y));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::ceil() const{

    return Vector2_t<T>(ceilf(x), ceilf(y));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::floor() const{

    return Vector2_t<T>(floorf(x), floorf(y));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::round() const{

    return Vector2_t<T>(roundf(x), roundf(y));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::clamp(const arithmetic auto & _min, const arithmetic auto & _max) const {
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
constexpr Vector2_t<T> Vector2_t<T>::dir_to(const Vector2_t<T> & b) const{
    return (b - *this).normalized();
}

template<arithmetic T>
constexpr T Vector2_t<T>::dist_to(const Vector2_t<T> & b) const{
    return (b - *this).length();
}

template<arithmetic T>
constexpr T Vector2_t<T>::dist_squared_to(const Vector2_t<T> & b) const{
    return (b - *this).length_squared();
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::reflect(const Vector2_t<T> & n) const {
    return 2.0f * n * this->dot(n) - *this;
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::bounce(const Vector2_t<T> & n) const {
    return -reflect(n);
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::lerp(const Vector2_t<T> & b, const arithmetic auto & _t) const{
    return *this * (1-_t)+b * _t;
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::slerp(const Vector2_t<T> & b, const arithmetic auto & _t) const{
    return lerp(b, sinf(PI / 2 * _t));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::posmod(const arithmetic auto & mod) const{
    return Vector2_t<T>( fmod(x, mod), fmod(y, mod));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::posmodv(const Vector2_t<T> & b) const{
    return Vector2_t<T>(fmod(x, b.x), fmod(y, b.y));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::project(const Vector2_t<T> & b) const{
    return (this->dot(b)) * b / b.length_squared();
}

template<arithmetic T>
constexpr T Vector2_t<T>::project(const T & rad) const{
    auto [s,c] = sincos(rad);
    return (this->x) * c + (this->y) * s;

}

template<arithmetic T>
constexpr bool Vector2_t<T>::is_equal_approx(const Vector2_t<T> & b) const{
    return ymd::is_equal_approx(x, b.x) && ymd::is_equal_approx(y, b.y);
}

template<arithmetic T>
constexpr bool Vector2_t<T>::has_point(const Vector2_t<auto> & _v) const{
    bool ret = true;
    Vector2_t<T> v = _v;

    if(x < 0) ret &= (x <= v.x && v.x <= 0);
    else ret &= (0 <= v.x && v.x <= x);
    if(y < 0) ret &= (y <= v.y && v.y <= 0);
    else ret &= (0 <= v.y && v.y <= y);

    return ret;
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::move_toward(const Vector2_t<T> & b, const arithmetic auto & delta) const{
    if (!is_equal_approx(b)){
        Vector2_t<T> d = b - *this;
        return *this + d.clampmax(delta);
    }
    return *this;
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::slide(const Vector2_t<T> & n) const {
    return *this - n * this->dot(n);
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::sign() const{
    return Vector2_t<T>(sign(x), sign(y));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::snapped(const Vector2_t<T> &by) const{
    return Vector2_t<T>(snap(x, by.x), snap(y, by.y));
}

template<arithmetic T>
constexpr __fast_inline Vector2_t<T> Vector2_t<T>::normalized() const{
    return *this * isqrt(this->length_squared());
}

template<arithmetic T>
constexpr __fast_inline T Vector2_t<T>::dot(const Vector2_t<T> & with) const{
    return (x*with.x + y*with.y);
}

template<arithmetic T>
constexpr __fast_inline T Vector2_t<T>::cross(const Vector2_t<T> & with) const{
    return (x*with.y - y*with.x);
}


template<arithmetic T>
constexpr __fast_inline Vector2_t<T> Vector2_t<T>::improduct(const Vector2_t<T> & b) const{
    return Vector2_t<T>(x*b.x - y*b.y, x*b.y + y*b.x);
}

template<arithmetic T>
constexpr __fast_inline Vector2_t<T> Vector2_t<T>::rotated(const T r) const{
    auto [s, c] = sincos(r);
    return this->improduct(Vector2_t<T>(c, s));
}

#define VECTOR2_COMPARE_IM_OPERATOR(op) \
\
template <arithmetic T, arithmetic U> \
constexpr __fast_inline bool operator op (const Vector2_t<T>& lhs, const U& rhs) { \
    T absrhs = static_cast<T>(abs(rhs)); \
    return lhs.length_squared() op (absrhs * absrhs); \
} \
\
template <arithmetic T, arithmetic U> \
constexpr __fast_inline bool operator op (const U& lhs, const Vector2_t<T>& rhs) { \
    T abslhs = static_cast<T>(abs(lhs)); \
    return (abslhs * abslhs) op rhs.length_squared(); \
}\
template <arithmetic T, arithmetic U> \
constexpr __fast_inline bool operator op (const Vector2_t<T>& lhs, const Vector2_t<U>& rhs) { \
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);\
}\


VECTOR2_COMPARE_IM_OPERATOR(<)
VECTOR2_COMPARE_IM_OPERATOR(<=)
VECTOR2_COMPARE_IM_OPERATOR(>)
VECTOR2_COMPARE_IM_OPERATOR(>=)
VECTOR2_COMPARE_IM_OPERATOR(==)
VECTOR2_COMPARE_IM_OPERATOR(!=)

template <arithmetic T, arithmetic U = T>
constexpr __fast_inline Vector2_t<T> operator +(const Vector2_t<T> &p_vector2, const Vector2_t<U> &d_vector2){
    Vector2_t<T> ret = p_vector2;
    ret += d_vector2;
    return ret;
}

template <arithmetic T, arithmetic U = T>
constexpr __fast_inline Vector2_t<T> operator -(const Vector2_t<T> &p_vector2, const Vector2_t<U> &d_vector2){
    Vector2_t<T> ret = p_vector2;
    ret -= d_vector2;
    return ret;
}

template <arithmetic T, arithmetic U = T>
constexpr __fast_inline Vector2_t<T> operator *(const arithmetic auto &lvalue, const Vector2_t<U> &p_vector2){
    Vector2_t<T> ret = p_vector2;
    ret *= lvalue;
    return ret;
}


template <arithmetic T,arithmetic U = T >
constexpr Vector2_t<T> operator/(const Vector2_t<T> &p_vector2, const Vector2_t<U> &d_vector2){
    Vector2_t<T> final = p_vector2;
    final /= d_vector2;
    return final;
}

template<arithmetic T>
Vector2_t() -> Vector2_t<T>;

}