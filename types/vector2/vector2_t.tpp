#include "../real.hpp"

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
template<typename U>
Vector2_t<T> Vector2_t<T>::clampmin(const U & _length) const{
    T length = static_cast<T>(_length);
    T l = this->length();
    return (l < length ? *this * length / l : *this);
}

template<typename T>
template<typename U>
Vector2_t<T> Vector2_t<T>::clampmax(const U & _length) const{
    T length = static_cast<T>(_length);
    T l = this->length();
    return (l > length ? *this * length / l : *this);
}


template<typename T>
template<typename U>
Vector2_t<T> Vector2_t<T>::clamp(const U & _min, const U & _max) const {
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
    return lerp(b, sinf(PI / 2 * t));
}

template<typename T>
Vector2_t<T> Vector2_t<T>::posmod(const T & mod) const{
    return Vector2_t<T>(fmod(x, mod), fmod(y, mod));
}

template<typename T>
Vector2_t<T> Vector2_t<T>::posmodv(const Vector2_t<T> & b) const{
    return Vector2_t<T>(fmod(x, b.x), fmod(y, b.y));
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
template<typename U>
bool Vector2_t<T>::has_point(const Vector2_t<U> & _v) const{
    Vector2_t<T> v = _v;
    bool ret = true;
    if(this.x < 0) ret &= (this.x <= v.x && v.x <= 0);
    else ret &= (0 <= v.x && v.x <= this.x);
    if(this.y < 0) ret &= (this.y <= v.y && v.y <= 0);
    else ret &= (0 <= v.y && v.y <= this.y);

    return ret;
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
    return Vector2_t<T>(sgn(x), sgn(y));
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
    T absrhs = static_cast<T>(abs(rhs)); \
    return lhs.length_squared() op (absrhs * absrhs); \
} \
\
template <typename T, typename U> \
__fast_inline bool operator op (const U& lhs, const Vector2_t<T>& rhs) { \
    T abslhs = static_cast<T>(abs(lhs)); \
    return abslhs * abslhs op rhs.length_squared(); \
}

VECTOR2_COMPARE_IM_OPERATOR(<)
VECTOR2_COMPARE_IM_OPERATOR(<=)
VECTOR2_COMPARE_IM_OPERATOR(>)
VECTOR2_COMPARE_IM_OPERATOR(>=)
VECTOR2_COMPARE_IM_OPERATOR(==)
VECTOR2_COMPARE_IM_OPERATOR(!=)

#define VECTOR2_ADD_SUB_MUL_OPERATOR(op) \
template <typename T, typename U> \
__fast_inline Vector2_t<T> operator op(const Vector2_t<T> &p_vector2, const U &rvalue){ \
    Vector2_t<T> final = p_vector2; \
    final op##= rvalue; \
    return final; \
}\
\
template <typename T, typename U> \
__fast_inline Vector2_t<T> operator op(const U &lvalue, const Vector2_t<T> &p_vector2){ \
    Vector2_t<T> final = p_vector2; \
    final op##= lvalue; \
    return final; \
}\
\
template <typename T, typename U> \
__fast_inline Vector2_t<T> operator op(const Vector2_t<T> &p_vector2, const Vector2_t<U> &d_vector2){ \
    Vector2_t<T> final = p_vector2; \
    final op##= d_vector2; \
    return final; \
}

VECTOR2_ADD_SUB_MUL_OPERATOR(+) 
VECTOR2_ADD_SUB_MUL_OPERATOR(-) 
VECTOR2_ADD_SUB_MUL_OPERATOR(*) 

#undef VECTOR2_ADD_SUB_MUL_OPERATOR

template <typename T, typename U>
Vector2_t<T> operator/(const Vector2_t<T> &p_vector2, const U &rvalue){
    Vector2_t<T> final = p_vector2;
    final /= rvalue;
    return final;
}

template <typename T, typename U>
Vector2_t<T> operator/(const Vector2_t<T> &p_vector2, const Vector2_t<U> &d_vector2){
    Vector2_t<T> final = p_vector2;
    final /= d_vector2;
    return final;
}