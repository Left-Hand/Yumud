#ifndef __FONT_HPP__

#define __FONT_HPP__

#include "../../vector2/vector2_t.hpp"
#include "font8x6.h"
#include "font16x8.h"

class Font{
protected:
    void ** res;
public:
    Vector2i size;
    Font(void ** _res, Vector2i _size):res(_res), size(_size){;}
    virtual bool getPixel(const char & chr, const Vector2i & offset) = 0;
};


class Font8x6:public Font{
public:
    Font8x6():Font((void **)font8x6_enc, Vector2i(6,8)){;}
    bool getPixel(const char & chr, const Vector2i & offset) override{
        if (!size.has_point(offset)) return false;
        return font8x6_enc[MAX(chr - ' ', 0)][offset.x] & (1 << offset.y);
    }
};

class Font16x8:public Font{
public:
    Font16x8():Font((void **)font16x8_enc, Vector2i(8,16)){;}
    bool getPixel(const char & chr, const Vector2i & offset) override{
        if (!size.has_point(offset)) return false;
        return font16x8_enc[MAX(chr - ' ', 0)][offset.y] & (1 << offset.x);
    }
};
#endif