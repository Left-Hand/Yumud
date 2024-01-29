#ifndef __VECTOR2I_H__

#define __VECTOR2I_H__

#include<cmath>
#include<cstdint>

class Vector2i
{
public:


    int32_t x = 0;
    int32_t y = 0;

    Vector2i(){x = 0;y = 0;}
    Vector2i(int32_t _x, int32_t _y){x = _x;y = _y;}
    ~Vector2i(){;}
    
    Vector2i operator+(const Vector2i & b) const;
    // Vector2i operator+(const Vector2 & b) const;
    Vector2i operator-(const Vector2i & b) const;
    Vector2i operator-() const;
    Vector2i operator*(const int32_t & n) const;
    Vector2i operator/(const int32_t & n) const;

	bool operator==(const Vector2i &b) const { return ((x == b.x) && (y == b.y));}
	bool operator!=(const Vector2i &b) const { return ((x != b.x) || (y != b.y));}

	bool operator<(const Vector2i &b) const { return x == b.x ? (y < b.y) : (x < b.x); }
	bool operator>(const Vector2i &b) const { return x == b.x ? (y > b.y) : (x > b.x); }
	bool operator<=(const Vector2i &b) const { return x == b.x ? (y <= b.y) : (x < b.x); }
	bool operator>=(const Vector2i &b) const { return x == b.x ? (y >= b.y) : (x > b.x); }

    // friend Vector2i operator*(const float & n, const Vector2i & a);
};



inline Vector2i Vector2i::operator+(const Vector2i & b) const{
    return Vector2i(x+b.x, y+b.y);
}

// inline Vector2i Vector2i::operator+(const Vector2 & b) const{
//     return Vector2i(x+(int32_t)b.x, y+(int32_t)b.y);
// }


inline Vector2i Vector2i::operator-(const Vector2i & b) const{
    return Vector2i(x-b.x, y-b.y);
}

inline Vector2i Vector2i::operator-() const{
    return Vector2i(-x, -y);
}

inline Vector2i Vector2i::operator*(const int32_t & n) const{
    return Vector2i(x*n, y*n);
}

inline Vector2i Vector2i::operator/(const int32_t & n) const{
    return Vector2i(x/n, y/n);
}

#endif