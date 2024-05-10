#ifndef __PACKED_IMAGE_HPP__

#define __PACKED_IMAGE_HPP__

#include "image.hpp"

using PackedBinary = uint8_t;

class PackedBinaryImage:public ImageWithData<Binary, PackedBinary>{
public:
    virtual void drawV8(){
        // for()
    }
    virtual void drawH8(){}

protected:
    PackedBinaryImage(PackedBinary * _data, const Vector2i & _size): ImageBasics<Binary>(_size), ImageWithData<Binary, PackedBinary>(_data, _size){;}

};

class HorizonBinaryImage : public PackedBinaryImage{
protected:
    void putPixel_Unsafe(const Vector2i & pos, const Binary & color) override{
        uint32_t point_index = (pos.y * size.x + pos.x);
        uint32_t data_index = point_index / 8;
        data[data_index] |= (PackedBinary)color << (point_index % 8);
    }
    void getPixel_Unsafe(const Vector2i & pos, Binary & color) override{
        uint32_t point_index = (pos.y * size.x + pos.x);
        uint32_t data_index = point_index / 8;
        color = data[data_index] & (1 << (point_index % 8));
    }
    void putTexture_Unsafe(const Rect2i & rect, const Binary * color_ptr) override{
        uint32_t i = 0;
        for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++, i++)
                putPixel_Unsafe(Vector2i(x,y), color_ptr[i]);
    }

    void putRect_Unsafe(const Rect2i & rect, const Binary & color) override{
        for(int x = rect.position.x; x < rect.position.x + rect.size.x; x++)
            for(int y = rect.position.y; y < rect.position.y + rect.size.y; y++)
                putPixel_Unsafe(Vector2i(x,y), color);
    }
public:
    HorizonBinaryImage(PackedBinary * _data, const Vector2i & _size): ImageBasics<Binary>(_size), PackedBinaryImage(_data, _size){;}
};

class VerticalBinaryImage : public PackedBinaryImage{

};

#endif