// #include "Vector2.hpp"

// Vector2 Vector2::abs() const{
//     return Vector2(fabs(x), fabs(y));
// }


// Vector2 Vector2::ceil() const{

//     return Vector2(ceilf(x), ceilf(y));
// }

// Vector2 Vector2::floor() const{

//     return Vector2(floorf(x), floorf(y));
// }

// Vector2 Vector2::round() const{

//     return Vector2(roundf(x), roundf(y));
// }

// Vector2 Vector2::clampmin(const float & length) const{
//     float l = this->length();
//     if (l < length){
//         return *this * length / l;
//     }
//     return *this;
// }

// Vector2 Vector2::clampmax(const float & length) const{
//     float l = this->length();
//     if (l > length){
//         return *this * length / l;
//     }
//     return *this;
// }

// Vector2 Vector2::clamp(const float & min, const float & max) const {
//     float l = this->length();
//     if (l > max){
//         return *this * max / l;
//     }
//     else if(l < min){
//         return *this * min / l;
//     }
//     return *this;
// }
// Vector2 Vector2::dir_to(const Vector2 & b) const{
//     return (b - *this).normalized();
// }
// float Vector2::dist_to(const Vector2 & b) const{
//     return (b - *this).length();
// }

// float Vector2::dist_squared_to(const Vector2 & b) const{
//     return (b - *this).length_squared();
// }

// float Vector2::angle_to(const Vector2 & to) const {
//     return (to.angle() - angle());
// }

// float Vector2::angle_to_point(const Vector2 & to) const {
//     return (to - *this).angle();
// }

// Vector2 Vector2::reflect(const Vector2 & n) const {
//     return 2.0f * n * this->dot(n) - *this;
// }

// Vector2 Vector2::bounce(const Vector2 & n) const {
//     return -reflect(n);
// }

// Vector2 Vector2::lerp(const Vector2 & b, const float & t) const{
//     return *this * (1-t)+b * t;
// }

// Vector2 Vector2::slerp(const Vector2 & b, const float & t) const{
//     return lerp(b, sin(1.5707963267948966192313216916398f*t));
// }

// Vector2 Vector2::posmod(const float & mod) const{
//     return Vector2(fmod(x, mod), fmod(y, mod));
// }

// Vector2 Vector2::posmodv(const Vector2 & b) const{
//     return Vector2(fmod(x, b.x), fmod(y, b.y));
// }

// Vector2 Vector2::project(const Vector2 & b) const{
//     return b * (dot(b) / b.length_squared());
// }

// bool Vector2::is_equal_approx(const Vector2 & b) const{
//     return is_equal_approx_f(x, b.x) && is_equal_approx_f(y, b.y);
// }

// Vector2 Vector2::move_toward(const Vector2 & b, const float & delta) const{
//     if (!is_equal_approx(b)){
//         Vector2 d = b - *this; 
//         return *this + d.clampmax(delta);
//     }
//     return *this;
// }

// Vector2 Vector2::slide(const Vector2 & n) const {
//     return *this - n * this->dot(n);
// }

// Vector2 Vector2::sign() const{
//     return Vector2(sgn(x), sgn(y));
// }

// Vector2 Vector2::snapped(const Vector2 &by) const{
//     return Vector2(snap(x, by.x), snap(y, by.y));
// }