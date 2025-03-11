#pragma once

#include "types/vector2/vector2.hpp"

namespace ymd{

class Font{
protected:

    Vector2i size;
    uint8_t scale;
    virtual bool _getpixel(const wchar_t chr, const Vector2i & offset) const = 0;
public:
    constexpr Font(Vector2i _size, uint8_t _scale = 1):size(_size), scale(_scale){;}
    bool getpixel(const wchar_t chr, const Vector2i & offset) const{
        return _getpixel(chr, {offset.x / scale, offset.y / scale});
    }

    void setScale(const uint8_t _scale){scale = _scale;}
    Vector2i getSize() const { return size * scale; }
};

}