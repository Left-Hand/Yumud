#pragma once

#include "image.hpp"


namespace ymd{

using PackedBinary = uint8_t;

class [[nodiscard]] HorizonBinaryImage final{
public:

    explicit HorizonBinaryImage(std::shared_ptr<PackedBinary[]> _data, const Vec2u & _size): 
        resource_(std::move(_data)),
        size_(_size){;}
    explicit HorizonBinaryImage(const Vec2u & _size): 
        HorizonBinaryImage(std::make_shared<PackedBinary[]>(size().x * size().y / 8), _size){;}

    void putseg_h8_unchecked(const Vec2u & pos, const uint8_t mask, const Binary color){
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t data_index = point_index / 8;
        if(data_index % 8){
            uint8_t & data_low = head_ptr()[data_index];
            uint8_t & data_high = head_ptr()[data_index + 1];
            uint16_t datum = (data_high << 8) | data_low; 
            const uint16_t shifted_mask = mask << (data_index % 8);
            // uint16_t presv = datum & (~shifted_mask);
            if(color.is_white()){
                datum |= shifted_mask;
            }else{
                datum &= (~shifted_mask); 
            }

            data_low = (datum & 0xFF);
            data_high = (datum >> 8);
        }else{
            uint8_t & datum = head_ptr()[data_index];
            // uint8_t presv = datum & (~mask);
            if(color.is_white()){
                datum |= mask;
            }else{
                datum &= (~mask); 
            }
        }
    }

    [[nodiscard]] constexpr Vec2u size() const {return size_;}
    [[nodiscard]] uint8_t * head_ptr() {return resource_.get();}
    [[nodiscard]] const uint8_t * head_ptr() const {return resource_.get();}

    [[nodiscard]] uint8_t & operator[](const size_t idx){
        return resource_[idx];
    }

    [[nodiscard]] uint8_t operator[](const size_t idx) const {
        return resource_[idx];
    }

    void putpixel_unchecked(const Vec2u & pos, const Binary color){
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t data_index = point_index / 8;
        uint8_t mask = 1 << (point_index % 8);
        if(color == Binary::from_white()){
            head_ptr()[data_index] |= mask;
        }else{
            head_ptr()[data_index] &= ~mask;
        }

    }
    void getpixel_unchecked(const Vec2u & pos, Binary & color) const{
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t data_index = point_index / 8;
        color = Binary::from_bool(head_ptr()[data_index] & (1 << (point_index % 8)));
    }
private:
    std::shared_ptr<uint8_t[]> resource_;
    Vec2u size_;

};

class [[nodiscard]] VerticalBinaryImage final{

public:
    explicit VerticalBinaryImage(std::shared_ptr<PackedBinary[]> _data, const Vec2u & _size): 
        resource_(std::move(_data)),
        size_(_size){;}
    explicit VerticalBinaryImage(const Vec2u & _size): 
        VerticalBinaryImage(std::make_shared<PackedBinary[]>(size().x * size().y / 8), _size){;}

    void putseg_v8_unchecked(const Vec2u & pos, const uint8_t mask, const Binary color){
        uint32_t data_index = pos.x + (pos.y / 8) * size().x; 
        if(pos.y % 8){
            uint16_t datum = (head_ptr()[data_index + size().x] << 8) | head_ptr()[data_index];
            uint16_t shifted_mask = mask << (pos.y % 8);
            if(color.is_white()){
                datum |= shifted_mask;
            }else{
                datum &= (~shifted_mask); 
            }
            head_ptr()[data_index] = datum & 0xFF;
            head_ptr()[data_index + size().x] = datum >> 8;
        }else{
            if(color.is_white()){
                head_ptr()[data_index] |= mask;
            }else{
                head_ptr()[data_index] &= (~mask);
            }
        }
    }

    [[nodiscard]] constexpr Vec2u size() const {return size_;}
    [[nodiscard]] uint8_t * head_ptr() {return resource_.get();}
    [[nodiscard]] const uint8_t * head_ptr() const {return resource_.get();}

    [[nodiscard]] uint8_t & operator[](const size_t idx){
        return resource_[idx];
    }

    [[nodiscard]] uint8_t operator[](const size_t idx) const {
        return resource_[idx];
    }

    void putpixel_unchecked(const Vec2u & pos, const Binary color){
        uint32_t data_index = pos.x + (pos.y / 8) * size().x; 
        uint8_t mask = (1 << (pos.y % 8));

        if(color.is_white()){
            head_ptr()[data_index] |= mask;
        }else{
            head_ptr()[data_index] &= (~mask);
        }
    }
    void getpixel_unchecked(const Vec2u & pos, Binary & color) const{
        uint32_t data_index = pos.x + (pos.y / 8) * size().x; 
        color = Binary::from_bool(head_ptr()[data_index] & (color.is_white() << (pos.y % 8)));
    }
private:
    std::shared_ptr<uint8_t[]> resource_;
    Vec2u size_;

};


}
