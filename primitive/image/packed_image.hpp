#pragma once

#include "image.hpp"


namespace ymd{

using PackedBinary = uint8_t;

class [[nodiscard]] HorizonBinaryImage final{
public:

    explicit HorizonBinaryImage(std::shared_ptr<PackedBinary[]> resource, const Vec2u16 _size): 
        resource_(std::move(resource)),
        size_(_size){;}

    explicit HorizonBinaryImage(const Vec2u16 _size): 
        HorizonBinaryImage(std::make_shared<PackedBinary[]>(size().x * size().y / 8), _size){;}

    void putseg_h8_unchecked(const Vec2u16 pos, const uint8_t mask, const Binary color){
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t pixel_index = point_index / 8;
        if(pixel_index % 8){
            uint8_t & pixel_low = head_ptr()[pixel_index];
            uint8_t & pixel_high = head_ptr()[pixel_index + 1];
            uint16_t datum = (pixel_high << 8) | pixel_low; 
            const uint16_t shifted_mask = mask << (pixel_index % 8);
            // uint16_t presv = datum & (~shifted_mask);
            if(color.is_white()){
                datum |= shifted_mask;
            }else{
                datum &= (~shifted_mask); 
            }

            pixel_low = (datum & 0xFF);
            pixel_high = (datum >> 8);
        }else{
            uint8_t & datum = head_ptr()[pixel_index];
            // uint8_t presv = datum & (~mask);
            if(color.is_white()){
                datum |= mask;
            }else{
                datum &= (~mask); 
            }
        }
    }

    [[nodiscard]] constexpr Vec2u16 size() const {return size_;}
    [[nodiscard]] uint8_t * head_ptr() {return resource_.get();}
    [[nodiscard]] const uint8_t * head_ptr() const {return resource_.get();}

    [[nodiscard]] uint8_t & operator[](const size_t idx){
        return resource_[idx];
    }

    [[nodiscard]] uint8_t operator[](const size_t idx) const {
        return resource_[idx];
    }

    void putpixel_unchecked(const Vec2u16 pos, const Binary color){
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t pixel_index = point_index / 8;
        uint8_t mask = 1 << (point_index % 8);
        if(color == Binary::white()){
            head_ptr()[pixel_index] |= mask;
        }else{
            head_ptr()[pixel_index] &= ~mask;
        }

    }
    void getpixel_unchecked(const Vec2u16 pos, Binary & color) const{
        uint32_t point_index = (pos.y * size().x + pos.x);
        uint32_t pixel_index = point_index / 8;
        color = Binary::from_bool(head_ptr()[pixel_index] & (1 << (point_index % 8)));
    }
private:
    std::shared_ptr<uint8_t[]> resource_;
    Vec2u16 size_;

};

class [[nodiscard]] VerticalBinaryImage final{

public:
    explicit VerticalBinaryImage(std::shared_ptr<PackedBinary[]> resource, const Vec2u16 _size): 
        resource_(std::move(resource)),
        size_(_size){;}

    explicit VerticalBinaryImage(const Vec2u16 _size): 
        VerticalBinaryImage(std::make_shared<PackedBinary[]>(size().x * size().y / 8), _size){;}

    void putseg_v8_unchecked(const Vec2u16 pos, const uint8_t mask, const Binary color){
        uint32_t pixel_index = pos.x + (pos.y / 8) * size().x; 
        if(pos.y % 8){
            uint16_t datum = (head_ptr()[pixel_index + size().x] << 8) | head_ptr()[pixel_index];
            uint16_t shifted_mask = mask << (pos.y % 8);
            if(color.is_white()){
                datum |= shifted_mask;
            }else{
                datum &= (~shifted_mask); 
            }
            head_ptr()[pixel_index] = datum & 0xFF;
            head_ptr()[pixel_index + size().x] = datum >> 8;
        }else{
            if(color.is_white()){
                head_ptr()[pixel_index] |= mask;
            }else{
                head_ptr()[pixel_index] &= (~mask);
            }
        }
    }

    [[nodiscard]] constexpr Vec2u16 size() const {return size_;}
    [[nodiscard]] uint8_t * head_ptr() {return resource_.get();}
    [[nodiscard]] const uint8_t * head_ptr() const {return resource_.get();}

    [[nodiscard]] uint8_t & operator[](const size_t idx){
        return resource_[idx];
    }

    [[nodiscard]] uint8_t operator[](const size_t idx) const {
        return resource_[idx];
    }

    void putpixel_unchecked(const Vec2u16 pos, const Binary color){
        uint32_t pixel_index = pos.x + (pos.y / 8) * size().x; 
        uint8_t mask = (1 << (pos.y % 8));

        if(color.is_white()){
            head_ptr()[pixel_index] |= mask;
        }else{
            head_ptr()[pixel_index] &= (~mask);
        }
    }

    void getpixel_unchecked(const Vec2u16 pos, Binary & color) const{
        uint32_t pixel_index = pos.x + (pos.y / 8) * size().x; 
        color = Binary::from_bool(head_ptr()[pixel_index] & (color.is_white() << (pos.y % 8)));
    }
private:
    std::shared_ptr<uint8_t[]> resource_;
    Vec2u16 size_;

};


}
