#pragma once

#include "sys/core/platform.h"
#include "sys/math/uint24_t.h"

#include <tuple>



#ifdef __cplusplus
#include "sys/math/real.hpp"

namespace ymd{
enum class ColorEnum:uint32_t{
    WHITE   = 0xFFFFFF,    // White color
    YELLOW  = 0xFFFF00,    // Yellow color
    BRRED   = 0xFF4500,    // Brown-red color
    PINK    = 0xFFC0CB,    // Pink color
    RED     = 0x0000FF,    // Red color
    BROWN   = 0xA52A2A,    // Brown color
    GRAY    = 0x808080,    // Gray color
    GBLUE   = 0xADD8E6,    // Light blue color
    GREEN   = 0x00FF00,    // Green color
    BLUE    = 0xFF0000,    // Blue color
    BLACK   = 0x000000     // Black color
};

enum class RgbType:uint8_t{
    _RGB332,
    _RGB565,
    _RGB888,
    _Binary,
    _HSV888,
    _LAB888,
    _XYZ888,
    _Grayscale,
    _sGrayscale,
};


#endif

struct RGB332;
struct RGB565;
struct RGB888;
struct Binary;
struct HSV888;
struct LAB888;
struct XYZ888;
struct Binary;
struct Grayscale;
struct sGrayscale;

struct RGB888 {

    uint8_t r;
    uint8_t g;
    uint8_t b;

#ifdef __cplusplus
public:
    __fast_inline constexpr RGB888() = default;

    __fast_inline constexpr RGB888(const ColorEnum & color): RGB888(int(color)){;}

    __fast_inline constexpr RGB888(const int _data): 
        r(_data),
        g(_data >> 8),
        b(_data >> 16)     
    {;}
    
    __fast_inline constexpr RGB888(const RGB888 & other): 
        r(other.r),
        g(other.g),
        b(other.b)
    {;} 

    RGB888(const HSV888 & other);

    __fast_inline constexpr RGB888(const uint8_t _r, const uint8_t _g, const uint8_t _b):r(_r), g(_g), b(_b){;}

    __fast_inline constexpr explicit operator uint24_t() const {return (uint24_t)(r | (g << 8) | (b << 16));}
#endif
};


// struct XYZ888 {

//     uint8_t x;
//     uint8_t y;
//     uint8_t z;

// #ifdef __cplusplus
// public:
//     __fast_inline constexpr XYZ888() = default;
    
//     __fast_inline constexpr XYZ888(const XYZ888 & other): 
//         x(other.x),
//         y(other.y),
//         z(other.z)
//     {;} 

//     XYZ888(const RGB888 & xyz); 

//     __fast_inline constexpr XYZ888(const uint8_t _x, const uint8_t _y, const uint8_t _z):x(_x), y(_y), z(_z){;}
//     __fast_inline constexpr explicit operator uint24_t() const {return (uint24_t)(x | (y << 8) | (z << 16));}
// #endif
// };

struct LAB888 {

    uint8_t l;
    int8_t a;
    int8_t b;

#ifdef __cplusplus
public:
    __fast_inline constexpr LAB888() = default;
    
    __fast_inline constexpr LAB888(const LAB888 & other): 
        l(other.l),
        a(other.a),
        b(other.b)
    {;}

    LAB888(const RGB888 & lab); 

    operator RGB888() const;
    
    __fast_inline constexpr LAB888(const uint8_t _l, const int8_t _a, const int8_t _b):l(_l), a(_a), b(_b){;}
    __fast_inline constexpr explicit operator uint24_t() const {return (uint24_t)(l | (a << 8) | (b << 16));}
#endif
};




struct RGB332{
    union{
        struct{
            uint8_t b : 2;
            uint8_t g : 3;
            uint8_t r : 3;
        };
        uint8_t data;
    };

#ifdef __cplusplus

    __fast_inline constexpr RGB332() : data(0){;}

    __fast_inline constexpr RGB332(const int & _data): data((uint8_t)_data){;}

    __fast_inline constexpr explicit RGB332(const uint8_t & _r, const uint8_t & _g, const uint8_t & _b): b(_b), g(_g), r(_r){;}

    __fast_inline constexpr explicit RGB332(const uint8_t & _data): data(_data){;}

    __fast_inline constexpr explicit operator uint8_t() const {return data;}

#endif
};



struct RGB565{
    uint16_t b : 5;
    uint16_t g : 6;
    uint16_t r : 5;

#ifdef __cplusplus

    __fast_inline constexpr RGB565() = default;

    RGB565(const HSV888 & other);
    __fast_inline constexpr RGB565(const ColorEnum & color):RGB565(RGB888(color)){;}

    __fast_inline constexpr RGB565(const RGB888 & rgb): b(rgb.b >> 3), g(rgb.g >> 2), r(rgb.r >> 3){;}

    
    __fast_inline constexpr RGB565(const Grayscale & gs);
    
    __fast_inline constexpr RGB565(const Binary & bn);

    __fast_inline constexpr RGB565(const uint8_t _r, const uint8_t _g, const uint8_t _b): b(_b & 0b11111), g(_g & 0b111111), r(_r & 0b11111){;}

    __fast_inline constexpr operator RGB888() const {
        return RGB888(r << 3, g << 2, b << 3);
    }


    __fast_inline constexpr explicit RGB565(const uint16_t data):
        b(data & 0x1f),
        g((data >> 5) & 0x3f),
        r((data >> 11) & 0x1f)
    {;}

    __fast_inline constexpr operator uint16_t() const {return uni(r,g,b);}

    __fast_inline constexpr RGB565 & operator = (const uint16_t data){
        uint8_t _r, _g, _b;
        ::std::tie(_r, _g, _b) = seprate(data);
        r = _r;
        g = _g;
        b = _b;
        return *this;
    }

    __fast_inline constexpr RGB565 & operator = (const RGB565 & rgb){
        memcpy(this, &rgb, sizeof(RGB565));
        return *this;
    }
private:
    static __fast_inline constexpr ::std::tuple<uint8_t, uint8_t, uint8_t>seprate(const uint16_t data){return {(data >> 11) & 0x1f, (data >> 5) & 0x3f, data & 0x1f};}
    static __fast_inline constexpr uint16_t uni(const uint8_t _r, const uint8_t _g, const uint8_t _b){return ((_r & 0x1f) << 11) | ((_g & 0x3f) << 5) | (_b & 0x1f);}
    
#endif
};




struct HSV888 {

    uint8_t h;
    uint8_t s;
    uint8_t v;

#ifdef __cplusplus
public:
    __fast_inline constexpr HSV888() = default;

    __fast_inline constexpr HSV888(const HSV888 & other) = default;
    HSV888(const RGB888 & other);
    HSV888(const RGB565 & other);

    __fast_inline constexpr HSV888(const int _data): 
        h(_data),
        s(_data >> 8),
        v(_data >> 16)
    {;}

    __fast_inline constexpr HSV888(const int _h, const int _s, const int _v):
        h(uint8_t(_h)), s(uint8_t(_s)), v(uint8_t(_v))
    {;}

    __fast_inline constexpr explicit operator uint24_t() const {return (uint24_t)(
        uint32_t(h) << 16 | uint32_t(s) << 8 | uint32_t(v)
    );}

#endif
};



struct ARGB32{

    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;

#ifdef __cplusplus

    __fast_inline constexpr ARGB32() : ARGB32(0){;}

    __fast_inline constexpr ARGB32(const uint32_t _data): 
        a(_data),
        r(_data >> 8),
        g(_data >> 16),
        b(_data >> 24)
    
    {;}

    __fast_inline constexpr explicit ARGB32(const uint8_t _a, const uint8_t _r, const uint8_t _g, const uint8_t _b):a(_a), r(_r), g(_g), b(_b){;}

    __fast_inline constexpr explicit operator uint32_t() const {return *reinterpret_cast<const uint32_t *>(this);}

#endif
};



struct Binary{
    uint8_t data;

    enum{
        WHITE   = 0xFF,  // White color
        BLACK   = 0x00   // Black color
    };
#ifdef __cplusplus
    __fast_inline constexpr Binary() : data(0){;}
    __fast_inline constexpr Binary(const bool _data): data(_data ? 0xff : 0x00){;}

    __fast_inline constexpr Binary(const RGB888 & rgb): data((rgb.r + rgb.g + rgb.b) > 128 * 3 ? 255 : 0){;}

    __fast_inline constexpr explicit operator uint8_t() const {return data;}

    __fast_inline constexpr operator bool() const {return data;}

    __fast_inline constexpr operator RGB888() const {return RGB888{data, data, data};}
#endif
};



struct Grayscale{
    uint8_t data;

    enum{
        WHITE   = 0xFF,  // White color
        BLACK   = 0x00   // Black color
    };
#ifdef __cplusplus
    __fast_inline constexpr Grayscale() : data(0){;}

    __fast_inline constexpr Grayscale(const uint8_t & _data): data(_data){;}


    __fast_inline constexpr Grayscale(const RGB565 & rgb);
    __fast_inline constexpr Grayscale & operator = (const uint8_t & _data){data = _data; return *this;}


    __fast_inline constexpr operator uint8_t() const {return data;}

    __fast_inline constexpr operator RGB888() const {return RGB888{data, data, data};}

    __fast_inline constexpr explicit operator bool() const {return data;}

    __fast_inline constexpr bool operator > (const Grayscale & other){return data > other.data;}

    __fast_inline constexpr bool operator < (const Grayscale & other){return data < other.data;}

    __fast_inline constexpr bool operator >= (const Grayscale & other){return data >= other.data;}

    __fast_inline constexpr bool operator <= (const Grayscale & other){return data <= other.data;}

    __fast_inline constexpr Binary to_bina(const Grayscale & threshold = Grayscale(128)){return Binary(data > (uint8_t)threshold);}
#endif
};


struct sGrayscale{
    int8_t data;

    enum{
        WHITE   = 127,  // White color
        BLACK   = 0x00   // Black color
    };
#ifdef __cplusplus
    __fast_inline constexpr sGrayscale() : data(0){;}

    __fast_inline constexpr sGrayscale(const int8_t & _data): data(_data){;}

    __fast_inline constexpr sGrayscale(const bool & bina): data(bina ? 127 : 0x00){;}

    __fast_inline constexpr sGrayscale & operator = (const uint8_t & _data){data = _data; return *this;}

    __fast_inline constexpr operator uint8_t() const {return data;}

    __fast_inline constexpr explicit operator bool() const {return data;}

    __fast_inline constexpr bool operator > (const auto & other){return data > other.data;}

    __fast_inline constexpr bool operator < (const auto & other){return data < other.data;}

    __fast_inline constexpr bool operator >= (const auto & other){return data >= other.data;}

    __fast_inline constexpr bool operator <= (const auto & other){return data <= other.data;}

    __fast_inline constexpr Binary to_bina(const int8_t & threshold){return Binary(ABS(data) > threshold);}

    __fast_inline constexpr Binary to_bina_signed(const int8_t & threshold){return Binary(data > threshold);}
#endif
};


typedef struct Binary Binary;
typedef struct Grayscale Grayscale;
typedef struct RGB565 RGB565;
typedef struct ARGB32 ARGB32;
typedef struct RGB888 RGB888;
typedef RGB888 RGB24;

typedef struct Grayscale Grayscale;
typedef struct sGrayscale sGrayscale;
typedef struct RGB332 RGB332;

#ifdef __cplusplus
__fast_inline constexpr Grayscale::Grayscale(const RGB565 & rgb):data(((rgb.r*77 + rgb.g*150 + rgb.b*29+128) >> 8)){;}

__fast_inline constexpr RGB565::RGB565(const Grayscale & gs): b((uint8_t)gs >> 3), g((uint8_t)gs >> 2), r((uint8_t)gs >> 3){;}

__fast_inline constexpr RGB565::RGB565(const Binary & bn): RGB565((bool)bn ? 0xffff : 0){;}


class OutputStream;


OutputStream & operator<<(OutputStream & os, const Binary & bn);

OutputStream & operator<<(OutputStream & os, const Grayscale & gs);

OutputStream & operator<<(OutputStream & os, const sGrayscale & sgs);

OutputStream & operator<<(OutputStream & os, const RGB565 & rgb);

OutputStream & operator<<(OutputStream & os, const RGB888 & rgb);

OutputStream & operator<<(OutputStream & os, const LAB888 & lab);

OutputStream & operator<<(OutputStream & os, const HSV888 & hsv);

template<typename T>
concept is_monochrome = ::std::is_same_v<T, Binary> or ::std::is_same_v<T, Grayscale> or ::std::is_same_v<T, sGrayscale>;

template<typename T>
concept is_rgb = ::std::is_same_v<T, RGB24> or ::std::is_same_v<T, RGB332> or ::std::is_same_v<T, RGB565> or ::std::is_same_v<T, RGB888> ;

template<typename T>
concept is_polychrome = is_rgb<T> or ::std::is_same_v<T, LAB888> or ::std::is_same_v<T, HSV888>;

template<typename T>
concept is_color = is_monochrome<T> or is_polychrome<T> or ::std::is_same_v<T, ColorEnum>;

}
#endif
