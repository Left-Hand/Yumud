#include "Vector2.hpp"

template<typename T>
Vector2_t<T> Vector2_t<T>::abs() const{
    return Vector2_t<T>(fabs(x), fabs(y));
}

template<typename T>
Vector2_t<T> Vector2_t<T>::ceil() const{

    return Vector2_t<T>(ceilf(x), ceilf(y));
}

template<typename T>
Vector2_t<T> Vector2_t<T>::floor() const{

    return Vector2_t<T>(floorf(x), floorf(y));
}

template<typename T>
Vector2_t<T> Vector2_t<T>::round() const{

    return Vector2_t<T>(roundf(x), roundf(y));
}

template<typename T>
Vector2_t<T> Vector2_t<T>::clampmin(const T & length) const{
    T l = this->length();
    if (l < length){
        return *this * length / l;
    }
    return *this;
}

template<typename T>
Vector2_t<T> Vector2_t<T>::clampmax(const T & length) const{
    T l = this->length();
    if (l > length){
        return *this * length / l;
    }
    return *this;
}

template<typename T>
Vector2_t<T> Vector2_t<T>::clamp(const T & min, const T & max) const {
    T l = this->length();
    if (l > max){
        return *this * max / l;
    }
    else if(l < min){
        return *this * min / l;
    }
    return *this;
}

template<typename T>
Vector2_t<T> Vector2_t<T>::dir_to(const Vector2_t<T> & b) const{
    return (b - *this).normalized();
}

template<typename T>
T Vector2_t<T>::dist_to(const Vector2_t<T> & b) const{
    return (b - *this).length();
}

template<typename T>
T Vector2_t<T>::dist_squared_to(const Vector2_t<T> & b) const{
    return (b - *this).length_squared();
}

template<typename T>
T Vector2_t<T>::angle_to(const Vector2_t<T> & to) const {
    return (to.angle() - angle());
}

template<typename T>
T Vector2_t<T>::angle_to_point(const Vector2_t<T> & to) const {
    return (to - *this).angle();
}

template<typename T>
Vector2_t<T> Vector2_t<T>::reflect(const Vector2_t<T> & n) const {
    return 2.0f * n * this->dot(n) - *this;
}

template<typename T>
Vector2_t<T> Vector2_t<T>::bounce(const Vector2_t<T> & n) const {
    return -reflect(n);
}

template<typename T>
Vector2_t<T> Vector2_t<T>::lerp(const Vector2_t<T> & b, const T & t) const{
    return *this * (1-t)+b * t;
}

template<typename T>
Vector2_t<T> Vector2_t<T>::slerp(const Vector2_t<T> & b, const T & t) const{
    return lerp(b, std::sinf(1.5707963267948966192313216916398f*t));
}

template<typename T>
Vector2_t<T> Vector2_t<T>::posmod(const T & mod) const{
    return Vector2_t<T>(std::fmod(x, mod), std::fmod(y, mod));
}

template<typename T>
Vector2_t<T> Vector2_t<T>::posmodv(const Vector2_t<T> & b) const{
    return Vector2_t<T>(std::fmod(x, b.x), std::fmod(y, b.y));
}

template<typename T>
Vector2_t<T> Vector2_t<T>::project(const Vector2_t<T> & b) const{
    return b * (dot(b) / b.length_squared());
}

template<typename T>
bool Vector2_t<T>::is_equal_approx(const Vector2_t<T> & b) const{
    return is_equal_approx_f(x, b.x) && is_equal_approx_f(y, b.y);
}

template<typename T>
Vector2_t<T> Vector2_t<T>::move_toward(const Vector2_t<T> & b, const T & delta) const{
    if (!is_equal_approx(b)){
        Vector2_t<T> d = b - *this; 
        return *this + d.clampmax(delta);
    }
    return *this;
}

template<typename T>
Vector2_t<T> Vector2_t<T>::slide(const Vector2_t<T> & n) const {
    return *this - n * this->dot(n);
}

template<typename T>
Vector2_t<T> Vector2_t<T>::sign() const{
    return Vector2_t<T>(std::sign, sgn(y));
}

template<typename T>
Vector2_t<T> Vector2_t<T>::snapped(const Vector2_t<T> &by) const{
    return Vector2_t<T>(snap(x, by.x), snap(y, by.y));
}

template<typename T>
__fast_inline Vector2_t<T> Vector2_t<T>::normalized() const{
    return *this/this->length();
}

template<typename T>
__fast_inline T Vector2_t<T>::dot(const Vector2_t<T> & with) const{
    return (x*with.x + y*with.y);
}

template<typename T>
__fast_inline T Vector2_t<T>::cross(const Vector2_t<T> & with) const{
    return (x*with.y - y*with.x);
}

template<typename T>
__fast_inline Vector2_t<T> Vector2_t<T>::improduct(const Vector2_t<T> & b) const{
    return Vector2_t<T>(x*b.x - y*b.y, x*b.y + y*b.x);
}

template<typename T>
__fast_inline Vector2_t<T> Vector2_t<T>::rotate(const T & r) const{
    return this->improduct(Vector2_t<T>(cos(r), sin(r)));
}

#define VECTOR2_COMPARE_IM_OPERATOR(op) \
\
template <typename T, typename U> \
__fast_inline bool operator op (const Vector2_t<T>& lhs, const U& rhs) { \
    using CommonType = typename std::common_type<T, U>::type; \
    U absrhs = std::abs(rhs); \
    return static_cast<CommonType>(lhs.length_squared()) op static_cast<CommonType>(absrhs * absrhs); \
} \
\
template <typename T, typename U> \
__fast_inline bool operator op (const U& lhs, const Vector2_t<T>& rhs) { \
    using CommonType = typename std::common_type<T, U>::type; \
    U abslhs = std::abs(lhs); \
    return static_cast<CommonType>(abslhs * abslhs) op static_cast<CommonType>(rhs.length_squared()); \
}

VECTOR2_COMPARE_IM_OPERATOR(<)
VECTOR2_COMPARE_IM_OPERATOR(<=)
VECTOR2_COMPARE_IM_OPERATOR(>)
VECTOR2_COMPARE_IM_OPERATOR(>=)
VECTOR2_COMPARE_IM_OPERATOR(==)
VECTOR2_COMPARE_IM_OPERATOR(!=)