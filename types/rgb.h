#ifndef __COLOR_H__

#define __COLOR_H__

#include "stdint.h"
#include "defines/comm_inc.h"
#include "uint24_t.h"

struct RGB232{
    union{
        struct{
            uint8_t b : 2;
            uint8_t g : 3;
            uint8_t r : 2;
        };
        uint8_t data;
    };

#ifdef __cplusplus

    __fast_inline RGB232() : data(0){;}
    
    __fast_inline RGB232(const int & _data): data((uint8_t)_data){;}

    __fast_inline explicit RGB232(const uint8_t & _r, const uint8_t & _g, const uint8_t & _b): b(_b), g(_g), r(_r){;}

    __fast_inline explicit RGB232(const uint8_t & _data): data(_data){;}
    
    __fast_inline explicit operator uint8_t() const {return data;}

#endif
};

typedef struct RGB232 RGB232;

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

    __fast_inline RGB888() : data(0){;}

    __fast_inline RGB888(const int & _data): data((uint24_t)_data){;}

    __fast_inline explicit RGB888(const uint8_t & _r, const uint8_t & _g, const uint8_t & _b):r(_r), g(_g), b(_b){;}

    __fast_inline explicit RGB888(const uint24_t & _data): data(_data){;}
    
    __fast_inline explicit operator uint24_t() const {return (uint24_t)data;}

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

#endif