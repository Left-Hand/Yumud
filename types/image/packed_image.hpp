#pragma once

#include "image.hpp"


namespace ymd{

using PackedBinary = uint8_t;

class PackedBinaryImage{
public:

protected:
    PackedBinaryImage(std::shared_ptr<PackedBinary[]> _data, const Vector2u & _size): 
        data_(std::move(_data)),
        size_(_size){;}
    PackedBinaryImage(const Vector2u & _size): 
        PackedBinaryImage(std::make_shared<PackedBinary[]>(size().x * size().y / 8), _size){;}

protected:
    std::shared_ptr<uint8_t[]> data_;
    Vector2u size_;

public:
    constexpr Vector2u size() const {return size_;}
    auto * get_data() {return data_.get();}
    const auto * get_data() const {return data_.get();}

    auto & operator[](const size_t idx){
        return data_[idx];
    }

    const auto & operator[](const size_t idx) const {
        return data_[idx];
    }
};

class HorizonBinaryImage final: public PackedBinaryImage{
public:
    void putpixel_unchecked(const Vector2u & pos, const Binary color){
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t data_index = point_index / 8;
        uint8_t mask = 1 << (point_index % 8);
        if(color){
            get_data()[data_index] |= mask;
        }else{
            get_data()[data_index] &= ~mask;
        }

    }
    void getpixel_unchecked(const Vector2u & pos, Binary & color) const{
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t data_index = point_index / 8;
        color = get_data()[data_index] & (1 << (point_index % 8));
    }


public:
    HorizonBinaryImage(std::shared_ptr<PackedBinary[]> _data, const Vector2u & _size): 
        PackedBinaryImage(_data, _size){;}
    HorizonBinaryImage(const Vector2u & _size): 
        PackedBinaryImage(std::make_shared<PackedBinary[]>(size().x * size().y / 8), _size){;}

    void putseg_h8_unchecked(const Vector2u & pos, const uint8_t mask, const Binary color){
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t data_index = point_index / 8;
        if(data_index % 8){
            uint16_t & datum = *(uint16_t *)&get_data()[data_index];
            uint16_t shifted_mask = mask << (data_index % 8);
            // uint16_t presv = datum & (~shifted_mask);
            if(color){
                datum |= shifted_mask;
            }else{
                datum &= (~shifted_mask); 
            }
        }else{
            uint8_t & datum = get_data()[data_index];
            // uint8_t presv = datum & (~mask);
            if(color){
                datum |= mask;
            }else{
                datum &= (~mask); 
            }
        }
    }

};

class VerticalBinaryImage final: public PackedBinaryImage{
public:
    void putpixel_unchecked(const Vector2u & pos, const Binary color){
        uint32_t data_index = pos.x + (pos.y / 8) * size().x; 
        uint8_t mask = (1 << (pos.y % 8));

        if(color){
            get_data()[data_index] |= mask;
        }else{
            get_data()[data_index] &= (~mask);
        }
    }
    void getpixel_unchecked(const Vector2u & pos, Binary & color) const{
        uint32_t data_index = pos.x + (pos.y / 8) * size().x; 
        color = Binary(get_data()[data_index] & (PackedBinary)color << (pos.y % 8));
    }
public:
    VerticalBinaryImage(std::shared_ptr<PackedBinary[]> _data, const Vector2u & _size): 
        PackedBinaryImage(_data, _size){;}
    VerticalBinaryImage(const Vector2u & _size): 
        PackedBinaryImage(std::make_shared<PackedBinary[]>(size().x * size().y / 8), _size){;}

    void putseg_v8_unchecked(const Vector2u & pos, const uint8_t mask, const Binary color){
        uint32_t data_index = pos.x + (pos.y / 8) * size().x; 
        if(pos.y % 8){
            uint16_t datum = (get_data()[data_index + size().x] << 8) | get_data()[data_index];
            uint16_t shifted_mask = mask << (pos.y % 8);
            if(color){
                datum |= shifted_mask;
            }else{
                datum &= (~shifted_mask); 
            }
            get_data()[data_index] = datum & 0xFF;
            get_data()[data_index + size().x] = datum >> 8;
        }else{
            if(color){
                get_data()[data_index] |= mask;
            }else{
                get_data()[data_index] &= (~mask);
            }
        }
    }
};

}
