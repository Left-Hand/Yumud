#ifndef __COLOR_H__

#define __COLOR_H__

#include "stdint.h"
#include "src/platform.h"
#include "uint24_t.h"

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

    __fast_inline constexpr RGB888(const int & _data): data((uint24_t)_data){;}

    __fast_inline constexpr explicit RGB888(const uint8_t & _r, const uint8_t & _g, const uint8_t & _b):r(_r), g(_g), b(_b){;}

    __fast_inline constexpr explicit RGB888(const uint24_t & _data): data(_data){;}

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

    __fast_inline constexpr ARGB32(const int & _data): data((uint32_t)_data){;}

    __fast_inline constexpr explicit ARGB32(const uint8_t & _a, const uint8_t & _r, const uint8_t & _g, const uint8_t & _b):a(_a), r(_r), g(_g), b(_b){;}

    __fast_inline constexpr explicit ARGB32(const uint32_t & _data): data(_data){;}

    __fast_inline constexpr explicit operator uint32_t() const {return data;}

#endif
};

typedef struct ARGB32 ARGB32;

struct Binary{
    uint8_t data;

    enum{
        WHITE   = 0x01,  // White color
        BLACK   = 0x00   // Black color
    };
#ifdef __cplusplus
    __fast_inline constexpr Binary() : data(0){;}

    __fast_inline constexpr Binary(const uint8_t & _data): data(_data){;}
    //bool will be implicitly converted to uint8_t, add the bool constructer will be ambiguous 

    __fast_inline explicit operator uint8_t() const {return data;}

    __fast_inline constexpr operator bool() const {return data;}
#endif
};

typedef struct Binary Binary;

struct Grayscale{
    uint8_t data;

    enum{
        WHITE   = 0xFF,  // White color
        BLACK   = 0x00   // Black color
    };
#ifdef __cplusplus
    __fast_inline constexpr Grayscale() : data(0){;}

    __fast_inline constexpr Grayscale(const uint8_t & _data): data(_data){;}

    __fast_inline constexpr Grayscale & operator = (const uint8_t & _data){data = _data; return *this;}

    __fast_inline constexpr operator uint8_t() const {return data;}

    __fast_inline constexpr explicit operator bool() const {return data;}

    __fast_inline constexpr bool operator > (const Grayscale & other){return data > other.data;}

    __fast_inline constexpr bool operator < (const Grayscale & other){return data < other.data;}

    __fast_inline constexpr bool operator >= (const Grayscale & other){return data >= other.data;}

    __fast_inline constexpr bool operator <= (const Grayscale & other){return data <= other.data;}

    __fast_inline constexpr Binary to_bina(const uint8_t threshold = 128){return Binary(data > threshold);}
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
        PINK    = 0XF81F,  // Pink color
        RED     = 0xF800,  // Red color
        BROWN   = 0XBC40,  // Brown color
        GRAY    = 0X8430,  // Gray color
        GBLUE   = 0X07FF,  // Light blue color
        GREEN   = 0x07E0,  // Green color
        BLUE    = 0x001F,  // Blue color
        BLACK   = 0x0000   // Black color
    };
#ifdef __cplusplus

    __fast_inline constexpr RGB565() : data(0){;}

    __fast_inline constexpr RGB565(const int & _data): data((uint16_t)_data){;}

    __fast_inline constexpr RGB565(const Grayscale & gs): b((uint8_t)gs >> 3), g((uint8_t)gs >> 2), r((uint8_t)gs >> 3){;}

    __fast_inline constexpr RGB565(const Binary & bn): RGB565((bool)bn ? 0xffff : 0){;}

    __fast_inline constexpr explicit RGB565(const uint8_t & _r, const uint8_t & _g, const uint8_t & _b): b(_b), g(_g), r(_r){;}

    __fast_inline constexpr explicit RGB565(const uint16_t & _data): data(_data){;}

    __fast_inline constexpr explicit operator uint16_t() const {return data;}

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

    __fast_inline explicit operator bool() const {return data;}

    __fast_inline constexpr bool operator > (const auto & other){return data > other.data;}

    __fast_inline constexpr bool operator < (const auto & other){return data < other.data;}

    __fast_inline constexpr bool operator >= (const auto & other){return data >= other.data;}

    __fast_inline constexpr bool operator <= (const auto & other){return data <= other.data;}

    __fast_inline constexpr Binary to_bina(const int8_t & threshold){return Binary(ABS(data) > threshold);}

    __fast_inline constexpr Binary to_bina_singed(const int8_t & threshold){return Binary(data > threshold);}
#endif
};

#endif