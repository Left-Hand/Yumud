#pragma once

#include "image.hpp"

namespace ymd{

using PackedBinary = uint8_t;

class PackedBinaryImage:public ImageWithData<Binary, PackedBinary>{
public:

protected:
    PackedBinaryImage(std::shared_ptr<PackedBinary[]> _data, const Vector2u & _size): ImageBasics(_size), ImageWithData<Binary, PackedBinary>(_data, _size){;}
    PackedBinaryImage(const Vector2u & _size): ImageBasics(_size), ImageWithData<Binary, PackedBinary>(std::make_shared<PackedBinary[]>(size().x * size().y / 8), _size){;}
};

class HorizonBinaryImage : public PackedBinaryImage{
public:
    void putpixel_unsafe(const Vector2u & pos, const Binary color) override{
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t data_index = point_index / 8;
        uint8_t mask = 1 << (point_index % 8);
        if(color){
            data[data_index] |= mask;
        }else{
            data[data_index] &= ~mask;
        }

    }
    void getpixel_unsafe(const Vector2u & pos, Binary & color) const override{
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t data_index = point_index / 8;
        color = data[data_index] & (1 << (point_index % 8));
    }


public:
    HorizonBinaryImage(std::shared_ptr<PackedBinary[]> _data, const Vector2u & _size): ImageBasics(_size), PackedBinaryImage(_data, _size){;}
    HorizonBinaryImage(const Vector2u & _size): ImageBasics(_size), PackedBinaryImage(std::make_shared<PackedBinary[]>(size().x * size().y / 8), _size){;}

    void putseg_h8_unsafe(const Vector2u & pos, const uint8_t mask, const Binary color) override{
        uint32_t point_index = (pos.y * size().x + pos.x);
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
    void putpixel_unsafe(const Vector2u & pos, const Binary color) override{
        uint32_t data_index = pos.x + (pos.y / 8) * size().x; 
        uint8_t mask = (1 << (pos.y % 8));

        if(color){
            data[data_index] |= mask;
        }else{
            data[data_index] &= (~mask);
        }
    }
    void getpixel_unsafe(const Vector2u & pos, Binary & color) const override{
        uint32_t data_index = pos.x + (pos.y / 8) * size().x; 
        color = Binary(data[data_index] & (PackedBinary)color << (pos.y % 8));
    }
public:
    VerticalBinaryImage(std::shared_ptr<PackedBinary[]> _data, const Vector2u & _size): ImageBasics(_size), PackedBinaryImage(_data, _size){;}
    VerticalBinaryImage(const Vector2u & _size): ImageBasics(_size), PackedBinaryImage(std::make_shared<PackedBinary[]>(size().x * size().y / 8), _size){;}

    void putseg_v8_unsafe(const Vector2u & pos, const uint8_t mask, const Binary color) override{
        uint32_t data_index = pos.x + (pos.y / 8) * size().x; 
        if(pos.y % 8){
            uint16_t datum = (data[data_index + size().x] << 8) | data[data_index];
            uint16_t shifted_mask = mask << (pos.y % 8);
            if(color){
                datum |= shifted_mask;
            }else{
                datum &= (~shifted_mask); 
            }
            data[data_index] = datum & 0xFF;
            data[data_index + size().x] = datum >> 8;
        }else{
            if(color){
                data[data_index] |= mask;
            }else{
                data[data_index] &= (~mask);
            }
        }
    }
};


}