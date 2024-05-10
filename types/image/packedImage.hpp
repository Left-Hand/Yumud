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

    PackedBinaryImage(PackedBinary * _data, const Vector2i & _size): ImageBasics<Binary>(_size), ImageWithData<Binary, PackedBinary>(_data, _size){;}

};

class HorizonBinaryImage : public PackedBinaryImage{
    void putPixel_Unsafe(const Vector2i & pos, const Binary & color) override{
        data[this->getArea().size.x * pos.y + pos.x] = color;
    }
    void getPixel_Unsafe(const Vector2i & pos, Binary & color) override{
        color = data[this->getArea().size.x * pos.y + pos.x];}
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
};

class VerticalBinaryImage : public PackedBinaryImage{

};

#endif