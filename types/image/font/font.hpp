#ifndef __FONT_HPP__

#define __FONT_HPP__

#include "../../vector2/vector2_t.hpp"
#include "font6x8.h"

class Font{
protected:
    void ** res;
public:
    Vector2i size;
    Font(void ** _res, Vector2i _size):res(_res), size(_size){;}
    virtual bool getPixel(const char & chr, const Vector2i & offset) = 0;
};


class Font6x8:public Font{
public:
    Font6x8():Font((void **)font6x8_enc, Vector2i(6,8)){;}
    bool getPixel(const char & chr, const Vector2i & offset) override{
        if (!size.has_point(offset)) return false;
        return font6x8_enc[MAX(chr - 32, 0)][offset.x] & (1 << offset.y);
    }
};
#endif