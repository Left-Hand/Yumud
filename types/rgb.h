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

#ifdef __cplusplus

    __fast_inline RGB332() : data(0){;}

    __fast_inline RGB332(const int & _data): data((uint8_t)_data){;}

    __fast_inline explicit RGB332(const uint8_t & _r, const uint8_t & _g, const uint8_t & _b): b(_b), g(_g), r(_r){;}

    __fast_inline explicit RGB332(const uint8_t & _data): data(_data){;}

    __fast_inline explicit operator uint8_t() const {return data;}

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

#endif
};

typedef struct RGB332 RGB332;

struct RGB565{
    union{
        struct{
            uint16_t b : 5;
            uint16_t g : 6;
            uint16_t r : 5;
        };
        uint16_t data;
    };

#ifdef __cplusplus

    __fast_inline RGB565() : data(0){;}

    __fast_inline RGB565(const int & _data): data((uint16_t)_data){;}

    __fast_inline explicit RGB565(const uint8_t & _r, const uint8_t & _g, const uint8_t & _b): b(_b), g(_g), r(_r){;}

    __fast_inline explicit RGB565(const uint16_t & _data): data(_data){;}

    __fast_inline explicit operator uint16_t() const {return data;}

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

#endif
};

typedef struct RGB565 RGB565;

struct RGB888 {
    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        uint24_t data;
    };

#ifdef __cplusplus
public:
    __fast_inline RGB888() : data(0){;}

    __fast_inline RGB888(const int & _data): data((uint24_t)_data){;}

    __fast_inline explicit RGB888(const uint8_t & _r, const uint8_t & _g, const uint8_t & _b):r(_r), g(_g), b(_b){;}

    __fast_inline explicit RGB888(const uint24_t & _data): data(_data){;}

    __fast_inline explicit operator uint24_t() const {return (uint24_t)data;}

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

    __fast_inline ARGB32() : data(0){;}

    __fast_inline ARGB32(const int & _data): data((uint32_t)_data){;}

    __fast_inline explicit ARGB32(const uint8_t & _a, const uint8_t & _r, const uint8_t & _g, const uint8_t & _b):a(_a), r(_r), g(_g), b(_b){;}

    __fast_inline explicit ARGB32(const uint32_t & _data): data(_data){;}

    __fast_inline explicit operator uint32_t() const {return data;}

#endif
};

typedef struct ARGB32 ARGB32;

#ifdef __cplusplus
typedef bool Binary;
#else
typedef uint8_t Binary;
#endif

typedef uint8_t Grayscale;

#endif