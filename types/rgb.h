#pragma once

#include "sys/core/platform.h"

#include "uint24_t.h"

#ifdef __cplusplus
#include "sys/math/real.hpp"

enum class ColorEnum:uint32_t{
    WHITE   = 0xFFFFFF,    // White color
    YELLOW  = 0xFFFF00,    // Yellow color
    BRRED   = 0xFF4500,    // Brown-red color
    PINK    = 0xFFC0CB,    // Pink color
    RED     = 0xFF0000,    // Red color
    BROWN   = 0xA52A2A,    // Brown color
    GRAY    = 0x808080,    // Gray color
    GBLUE   = 0xADD8E6,    // Light blue color
    GREEN   = 0x008000,    // Green color
    BLUE    = 0x0000FF,    // Blue color
    BLACK   = 0x000000     // Black color
};


#endif




struct RGB332{
    union{
        struct{
            uint8_t b : 2;
            uint8_t g : 3;
            uint8_t r : 3;
        };
        uint8_t data;
    };

    enum {
        BLACK   = 0x00,    // Black color
        RED     = 0xE0,    // Red color
        GREEN   = 0x1C,    // Green color
        BLUE    = 0x03,    // Blue color
        YELLOW  = 0xFC,    // Yellow color
        MAGENTA = 0xE3,    // Magenta color
        CYAN    = 0x1F,    // Cyan color
        WHITE   = 0xFF     // White color
    };
#ifdef __cplusplus

    __fast_inline constexpr RGB332() : data(0){;}

    __fast_inline constexpr RGB332(const int & _data): data((uint8_t)_data){;}

    __fast_inline constexpr explicit RGB332(const uint8_t & _r, const uint8_t & _g, const uint8_t & _b): b(_b), g(_g), r(_r){;}

    __fast_inline constexpr explicit RGB332(const uint8_t & _data): data(_data){;}

    __fast_inline constexpr explicit operator uint8_t() const {return data;}

#endif
};

typedef struct RGB332 RGB332;



struct RGB888 {
    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        uint24_t data;
    };

    enum {
        WHITE   = 0xFFFFFF,    // White color
        YELLOW  = 0xFFFF00,    // Yellow color
        BRRED   = 0xFF4500,    // Brown-red color
        PINK    = 0xFFC0CB,    // Pink color
        RED     = 0xFF0000,    // Red color
        BROWN   = 0xA52A2A,    // Brown color
        GRAY    = 0x808080,    // Gray color
        GBLUE   = 0xADD8E6,    // Light blue color
        GREEN   = 0x008000,    // Green color
        BLUE    = 0x0000FF,    // Blue color
        BLACK   = 0x000000     // Black color
    };
#ifdef __cplusplus
public:
    __fast_inline constexpr RGB888() : data(0){;}

    __fast_inline constexpr RGB888(const ColorEnum & color): data(static_cast<uint32_t>(color)){;}

    __fast_inline constexpr RGB888(const RGB888 & other): data(other.data){;}

    __fast_inline constexpr RGB888(const int _data): data((uint24_t)_data){;}

    __fast_inline constexpr RGB888(const uint8_t _r, const uint8_t _g, const uint8_t _b):r(_r), g(_g), b(_b){;}

    __fast_inline constexpr explicit RGB888(const uint24_t _data): data(_data){;}

    __fast_inline constexpr explicit operator uint24_t() const {return (uint24_t)data;}
#endif
};

typedef struct RGB888 RGB888;
typedef RGB888 RGB24;


struct ARGB32{
    union {
        struct {
            uint8_t a;
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        uint32_t data;
    };

#ifdef __cplusplus

    __fast_inline constexpr ARGB32() : data(0){;}

    __fast_inline constexpr ARGB32(const int _data): data((uint32_t)_data){;}

    __fast_inline constexpr explicit ARGB32(const uint8_t _a, const uint8_t _r, const uint8_t _g, const uint8_t _b):a(_a), r(_r), g(_g), b(_b){;}

    __fast_inline constexpr explicit ARGB32(const uint32_t _data): data(_data){;}

    __fast_inline constexpr explicit operator uint32_t() const {return data;}

#endif
};

typedef struct ARGB32 ARGB32;

struct Binary{
    uint8_t data;

    enum{
        WHITE   = 0xFF,  // White color
        BLACK   = 0x00   // Black color
    };
#ifdef __cplusplus
    __fast_inline constexpr Binary() : data(0){;}
    // __fast_inline constexpr Binary(Binary & other) : data(other.data){;}
    // __fast_inline constexpr Binary(Binary && other) : data(other.data){;}
    // __fast_inline constexpr Binary(const uint8_t & _data): data(_data){;}
    __fast_inline constexpr Binary(const bool _data): data(_data ? 0xff : 0x00){;}
    //bool will be implicitly converted to uint8_t, add the bool constructer will be ambiguous 
    __fast_inline constexpr Binary(const RGB888 & rgb): data((rgb.r + rgb.g + rgb.b) > 128 * 3 ? 255 : 0){;}

    __fast_inline constexpr explicit operator uint8_t() const {return data;}

    __fast_inline constexpr operator bool() const {return data;}

    __fast_inline constexpr operator RGB888() const {return RGB888{data, data, data};}
#endif
};

typedef struct Binary Binary;

struct RGB565;

struct Grayscale{
    uint8_t data;

    enum{
        WHITE   = 0xFF,  // White color
        BLACK   = 0x00   // Black color
    };
#ifdef __cplusplus
    __fast_inline constexpr Grayscale() : data(0){;}

    __fast_inline constexpr Grayscale(const uint8_t & _data): data(_data){;}

    __fast_inline constexpr Grayscale(const real_t & _data): data(int(_data * 256)){;}

    __fast_inline constexpr Grayscale(const RGB565 & rgb);
    __fast_inline constexpr Grayscale & operator = (const uint8_t & _data){data = _data; return *this;}


    __fast_inline constexpr operator uint8_t() const {return data;}

    __fast_inline constexpr operator RGB888() const {return RGB888{data, data, data};}

    __fast_inline constexpr explicit operator bool() const {return data;}

    __fast_inline constexpr bool operator > (const Grayscale & other){return data > other.data;}

    __fast_inline constexpr bool operator < (const Grayscale & other){return data < other.data;}

    __fast_inline constexpr bool operator >= (const Grayscale & other){return data >= other.data;}

    __fast_inline constexpr bool operator <= (const Grayscale & other){return data <= other.data;}

    __fast_inline constexpr Binary to_bina(const Grayscale & threshold = 128){return Binary(data > (uint8_t)threshold);}
#endif
};



typedef struct Grayscale Grayscale;

struct RGB565{
    union{
        struct{
            uint16_t b : 5;
            uint16_t g : 6;
            uint16_t r : 5;
        };
        uint16_t data;
    };

    enum{
        WHITE   = 0xFFFF,  // White color
        YELLOW  = 0xFFE0,  // Yellow color
        BRRED   = 0XFC07,  // Brown-red color
        PINK    = 0XFE19,  // Pink color
        RED     = 0xF800,  // Red color
        BROWN   = 0XBC40,  // Brown color
        GRAY    = 0X8430,  // Gray color
        GBLUE   = 0X07FF,  // Light blue color
        GREEN   = 0x07E0,  // Green color
        BLUE    = 0x001F,  // Blue color
        BLACK   = 0x0000,   // Black color
        ORANGE = 0xFFA5,
        CORAL = 0xf410,
        AQUA = 0x67FC,
        FUCHSIA = 0xf014
    };
#ifdef __cplusplus

    __fast_inline constexpr RGB565() : data(0){;}

    __fast_inline constexpr RGB565(const ColorEnum & color):RGB565(RGB888(color)){;}

    __fast_inline constexpr RGB565(const int _data): data((uint16_t)_data){;}

    __fast_inline constexpr RGB565(const Grayscale gs): b((uint8_t)gs >> 3), g((uint8_t)gs >> 2), r((uint8_t)gs >> 3){;}

    __fast_inline constexpr RGB565(const RGB888 rgb): b(rgb.b >> 3), g(rgb.g >> 2), r(rgb.r >> 3){;}

    __fast_inline constexpr RGB565(const real_t _r, const real_t _g, const real_t _b): b(int(_b * 32)), g(int(_g * 64)), r(int(_r * 32)){;}

    __fast_inline constexpr RGB565(const Binary bn): RGB565((bool)bn ? 0xffff : 0){;}

    __fast_inline constexpr explicit RGB565(const uint8_t _r, const uint8_t _g, const uint8_t _b): b(_b), g(_g), r(_r){;}

    __fast_inline constexpr explicit RGB565(const uint16_t _data): data(_data){;}

    __fast_inline constexpr operator uint16_t() const {return data;}

    __fast_inline constexpr RGB565 & operator = (const uint16_t & _data){data = _data; return *this;}

#endif
};

typedef struct RGB565 RGB565;

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

#ifdef __cpluscplus
__fast_inline constexpr GrayScale::Grayscale(const RGB565 & rgb):data(((rgb.r*77 + rgb.g*150 + rgb.b*29+128) >> 8)){;}
#endif


#ifdef __cplusplus

template<typename T>
concept monochrome = std::is_same_v<T, Binary> || std::is_same_v<T, Grayscale>;


#endif
