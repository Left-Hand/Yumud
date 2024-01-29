// #ifndef __VECTOR2_H_

// #define __VECTOR2_H_


// class Vector2
// {

// public:    

//     float x = 0.0f;
//     float y = 0.0f;
//     Vector2();
//     Vector2(float _x, float _y);
//     ~Vector2();

//     Vector2 abs() const;
//     float angle() const {return atan2f(y, x);}
//     float angle_to(const Vector2 & to) const;
//     float angle_to_point(const Vector2 & to) const;
//     float aspect() const {return (!!y) ? x/y : .0f;}
//     Vector2 bounce(const Vector2 &n) const;
//     Vector2 ceil() const;
//     Vector2 clampmin(const float & length) const;
//     Vector2 clampmax(const float & length) const;
//     Vector2 clamp(const float & min, const float & max) const;
//     float cross(const Vector2 & with) const;
//     float dot(const Vector2 & with) const;
//     Vector2 dir_to(const Vector2 & b) const;
//     float dist_to(const Vector2 & b) const;
//     float dist_squared_to(const Vector2 & b) const;
//     Vector2 floor() const;
//     bool is_equal_approx(const Vector2 & v) const;
//     bool is_normalized() const {return (fabs(x*x + y*y - 1.0) <= CMP_EPSILON);}
//     float length() const {return sqrt(x*x + y*y);}
//     float length_squared() const {return (x*x + y*y);}
//     Vector2 lerp(const Vector2 & b, const float & t) const;
//     Vector2 move_toward(const Vector2 & to, const float & delta) const;
//     Vector2 normalized() const;
//     Vector2 posmod(const float & mod) const;
//     Vector2 posmodv(const Vector2 & modv) const;
//     Vector2 project(const Vector2 & b) const;
//     Vector2 reflect(const Vector2 & n) const;
//     Vector2 round() const;
//     Vector2 sign() const;
//     Vector2 slerp(const Vector2 & b, const float & t) const;
//     Vector2 slide(const Vector2  & n) const;
//     Vector2 snapped(const Vector2 & by) const;
//     Vector2 improduct(const Vector2 & b) const;
//     Vector2 rotate(const float & r)const;


//     Vector2 operator+(const Vector2 & b) const;
//     Vector2 operator+=(const Vector2 & b);
//     Vector2 operator-(const Vector2 & b) const;
//     Vector2 operator-=(const Vector2 & b);
//     Vector2 operator-() const;

// 	Vector2 operator*(const Vector2 &b) const;
//     Vector2 operator*(const float & n) const;
//     void operator*=(const float &n){*this = *this * n;}
// 	void operator*=(const Vector2 &b) { *this = *this * b; }

//     Vector2 operator/(const float & n) const;
//     Vector2 operator/(const Vector2 &b) const;
//     void operator/=(const float &n);
// 	void operator/=(const Vector2 &b) { *this = *this / b; }
// 	bool operator==(const Vector2 &b) const { return ((x == b.x) && (y == b.y));}
// 	bool operator!=(const Vector2 &b) const { return ((x != b.x) || (y != b.y));}

// 	bool operator<(const Vector2 &b) const { return x == b.x ? (y < b.y) : (x < b.x); }
// 	bool operator>(const Vector2 &b) const { return x == b.x ? (y > b.y) : (x > b.x); }
// 	bool operator<=(const Vector2 &b) const { return x == b.x ? (y <= b.y) : (x < b.x); }
// 	bool operator>=(const Vector2 &b) const { return x == b.x ? (y >= b.y) : (x > b.x); }

//     friend Vector2 operator*(const float & n, const Vector2 & a){return (Vector2(a.x*n, a.y*n));}


// };

// inline Vector2::Vector2()
// {
//     x = 0.0;
//     y = 0.0;
// }

// inline Vector2::Vector2(float _x, float _y){
//     x = _x;
//     y = _y;
// }

// inline Vector2::~Vector2()
// {
// }

// inline Vector2 Vector2::operator+(const Vector2 & b) const{
//     return Vector2(x+b.x, y+b.y);
// }

// inline Vector2 Vector2::operator+=(const Vector2 & b){
//     *this = *this + b;
//     return *this;
// }

// inline Vector2 Vector2::operator-(const Vector2 & b) const{
//     return Vector2(x-b.x, y-b.y);
// }

// inline Vector2 Vector2::operator-=(const Vector2 & b){
//     *this = *this - b;
//     return *this;
// }


// inline Vector2 Vector2::operator-() const{
//     return Vector2(-x, -y);
// }


// inline Vector2 Vector2::operator*(const float & n) const{
//     return Vector2(x*n, y*n);
// }

// inline Vector2 Vector2::operator*(const Vector2 &b) const{
//     return Vector2(x*b.x, y*b.y);
// }

// // void Vector2::operator*=(const float &n){
// //     x *= n;
// //     y *= n;
// // }

// inline Vector2 Vector2::operator/(const float & n) const{
//     return Vector2(x/n, y/n);
// }

// inline Vector2 Vector2::operator/(const Vector2 &b) const{
//     return Vector2(x/b.x, y/b.y);
// }

// inline void Vector2::operator/=(const float &n){
//     x /= n;
//     y /= n;
// }

// inline Vector2 Vector2::normalized() const{
//     return *this/this->length();
// }

// inline float Vector2::dot(const Vector2 & with) const{
//     return (x*with.x + y*with.y);
// }

// inline float Vector2::cross(const Vector2 & with) const{
//     return (x*with.y - y*with.x);
// }

// inline Vector2 Vector2::improduct(const Vector2 & b) const{
//     return Vector2(x*b.x - y*b.y, x*b.y + y*b.x);
// }

// inline Vector2 Vector2::rotate(const float & r) const{
//     return this->improduct(Vector2(cos(r), sin(r)));
// }
// #endif
