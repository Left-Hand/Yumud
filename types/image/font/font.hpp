#pragma once

#include "types/vectors/vector2/vector2.hpp"

namespace ymd{

class Font{
protected:

    Vector2i size;
    uint8_t scale;
    virtual bool _getpixel(const wchar_t chr, const Vector2i & offset) const = 0;
public:
    constexpr Font(Vector2i _size, uint8_t _scale = 1):size(_size), scale(_scale){;}
    bool get_pixel(const wchar_t chr, const Vector2i & offset) const{
        return _getpixel(chr, {offset.x / scale, offset.y / scale});
    }

    void set_scale(const uint8_t _scale){scale = _scale;}
    Vector2i get_size() const { return size * scale; }
};

}