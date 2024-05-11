#ifndef __PACKED_IMAGE_HPP__

#define __PACKED_IMAGE_HPP__

#include "image.hpp"

using PackedBinary = uint8_t;

class PackedBinaryImage:public ImageWithData<Binary, PackedBinary>{
public:

protected:
    PackedBinaryImage(PackedBinary * _data, const Vector2i & _size): ImageBasics<Binary>(_size), ImageWithData<Binary, PackedBinary>(_data, _size){;}

};

class HorizonBinaryImage : public PackedBinaryImage{
public:
    void putpixel_unsafe(const Vector2i & pos, const Binary & color) override{
        uint32_t point_index = (pos.y * size.x + pos.x);
        uint32_t data_index = point_index / 8;
        uint8_t mask = 1 << (point_index % 8);
        if(color){
            data[data_index] |= mask;
        }else{
            data[data_index] &= ~mask;
        }

    }
    void getpixel_unsafe(const Vector2i & pos, Binary & color) override{
        uint32_t point_index = (pos.y * size.x + pos.x);
        uint32_t data_index = point_index / 8;
        color = Binary(data[data_index] & (1 << (point_index % 8)));
    }


public:
    HorizonBinaryImage(PackedBinary * _data, const Vector2i & _size): ImageBasics<Binary>(_size), PackedBinaryImage(_data, _size){;}

    void putsegh8(const Vector2i & pos, const uint8_t & mask, const Binary & color) override{
        uint32_t point_index = (pos.y * size.x + pos.x);
        uint32_t data_index = point_index / 8;
        if(data_index % 8){
            uint16_t & datum = *(uint16_t *)&data[data_index];
            uint16_t shifted_mask = mask << (data_index % 8);
            // uint16_t presv = datum & (~shifted_mask);
            if(color){
                datum |= shifted_mask;
            }else{
                datum &= (~shifted_mask); 
            }
        }else{
            uint8_t & datum = data[data_index];
            // uint8_t presv = datum & (~mask);
            if(color){
                datum |= mask;
            }else{
                datum &= (~mask); 
            }
        }
    }
};

class VerticalBinaryImage : public PackedBinaryImage{
public:
    void putpixel_unsafe(const Vector2i & pos, const Binary & color) override{
        uint32_t data_index = pos.x + (pos.y / 8) * size.x; 
        uint8_t mask = (1 << (pos.y % 8));

        if(color){
            data[data_index] |= mask;
        }else{
            data[data_index] &= (~mask);
        }
    }
    void getpixel_unsafe(const Vector2i & pos, Binary & color) override{
        uint32_t data_index = pos.x + (pos.y / 8) * size.x; 
        color = Binary(data[data_index] & (PackedBinary)color << (pos.y % 8));
    }
public:
    VerticalBinaryImage(PackedBinary * _data, const Vector2i & _size): ImageBasics<Binary>(_size), PackedBinaryImage(_data, _size){;}

    virtual void putsegv8(const Vector2i & pos, const uint8_t & mask, const Binary & color){
        uint32_t data_index = pos.x + (pos.y / 8) * size.x; 
        if(pos.y % 8){
            uint16_t datum = (data[data_index + size.x] << 8) | data[data_index];
            uint16_t shifted_mask = mask << (pos.y % 8);
            if(color){
                datum |= shifted_mask;
            }else{
                datum &= (~shifted_mask); 
            }
        }else{
            if(color){
                data[data_index] |= mask;
            }else{
                data[data_index] &= (~mask);
            }
        }
    }
};

#endif