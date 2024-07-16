#include "../sys/core/platform.h"

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
constexpr Vector2_t<T> Vector2_t<T>::clampmin(const auto & _length) const{
    T length = static_cast<T>(_length);
    T l = this->length();
    return (l < length ? *this * length / l : *this);
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::clampmax(const auto & _length) const{
    T length = static_cast<T>(_length);
    T l = this->length();
    return (l > length ? *this * length / l : *this);
}


template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::clamp(const auto & _min, const auto & _max) const {
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
constexpr T Vector2_t<T>::angle_to(const Vector2_t<T> & to) const {
    return (to.angle() - angle());
}

template<arithmetic T>
constexpr T Vector2_t<T>::angle_to_point(const Vector2_t<T> & to) const {
    return (to - *this).angle();
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
constexpr Vector2_t<T> Vector2_t<T>::lerp(const Vector2_t<T> & b, const T & _t) const{
    return *this * (1-_t)+b * _t;
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::slerp(const Vector2_t<T> & b, const T & _t) const{
    return lerp(b, sinf(PI / 2 * _t));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::posmod(const T & mod) const{
    return Vector2_t<T>(::fmod(x, mod), ::fmod(y, mod));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::posmodv(const Vector2_t<T> & b) const{
    return Vector2_t<T>(::fmod(x, b.x), ::fmod(y, b.y));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::project(const Vector2_t<T> & b) const{
    return b * (dot(b) / b.length_squared());
}

template<arithmetic T>
constexpr bool Vector2_t<T>::is_equal_approx(const Vector2_t<T> & b) const{
    return is_equal_approx_f(x, b.x) && is_equal_approx_f(y, b.y);
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
constexpr Vector2_t<T> Vector2_t<T>::move_toward(const Vector2_t<T> & b, const T & delta) const{
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
    return Vector2_t<T>(::sign(x), ::sign(y));
}

template<arithmetic T>
constexpr Vector2_t<T> Vector2_t<T>::snapped(const Vector2_t<T> &by) const{
    return Vector2_t<T>(snap(x, by.x), snap(y, by.y));
}

template<arithmetic T>
constexpr __fast_inline Vector2_t<T> Vector2_t<T>::normalized() const{
    return *this/this->length();
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
    return this->improduct(Vector2_t<T>(::cos(r), ::sin(r)));
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

template <arithmetic T, arithmetic U>
constexpr __fast_inline Vector2_t<T> operator +(const Vector2_t<T> &p_vector2, const Vector2_t<U> &d_vector2){
    Vector2_t<T> ret = p_vector2;
    ret += d_vector2;
    return ret;
}

template <arithmetic T, arithmetic U>
constexpr __fast_inline Vector2_t<T> operator -(const Vector2_t<T> &p_vector2, const Vector2_t<U> &d_vector2){
    Vector2_t<T> ret = p_vector2;
    ret -= d_vector2;
    return ret;
}

template <arithmetic T>
constexpr __fast_inline Vector2_t<T> operator *(const auto &lvalue, const Vector2_t<T> &p_vector2){
    Vector2_t<T> ret = p_vector2;
    ret *= lvalue;
    return ret;
}


template <arithmetic T >
constexpr Vector2_t<T> operator/(const Vector2_t<T> &p_vector2, const Vector2_t<auto> &d_vector2){
    Vector2_t<T> final = p_vector2;
    final /= d_vector2;
    return final;
}