#pragma once

#include <cstdint>
#include <span>
#include <cstddef>

namespace ymd::robots::vesc::can{

struct [[nodiscard]] BytesFiller final{
public:
    constexpr explicit BytesFiller(std::span<uint8_t> bytes):
        bytes_(bytes){;}

    constexpr __inline 
    void push_be_u8(const uint16_t int_val) {
        push_byte(int_val);
    }

    constexpr __inline 
    void push_be_u16(const uint16_t int_val) {
        push_byte(int_val >> 8);
        push_byte(int_val & 0xFF);
    }

    constexpr __inline
    void push_be_i16(const int16_t int_val) { 
        return push_be_u16(std::bit_cast<uint16_t>(int_val));
    }

    constexpr __inline 
    void push_be_u32(const uint32_t int_val){
        push_be_u16(int_val >> 16);
        push_be_u16(int_val & 0xFFFF);
    }

    constexpr __inline 
    void push_be_i32(const int32_t int_val){
        push_be_u16(int_val >> 16);
        push_be_u16(int_val & 0xFFFF);
    }

    [[nodiscard]] constexpr bool is_full() const {
        return pos_ == bytes_.size();
    }

    [[nodiscard]] constexpr size_t size() const {
        return pos_;
    }
private:
    std::span<uint8_t> bytes_;
    size_t pos_ = 0;

    constexpr __inline 
    void push_byte_unchecked(const uint8_t byte){ 
        bytes_[pos_++] = byte;
    }

    template<size_t Extents>
    constexpr __inline 
    void push_bytes_unchecked(const std::span<const uint8_t, Extents> bytes){ 
        if constexpr(Extents == std::dynamic_extent){
            // #pragma GCC unroll(4)
            for(size_t i = 0; i < bytes.size(); i++){
                push_byte(bytes[i]);
            }
        }else{
            #pragma GCC unroll(4)
            for(size_t i = 0; i < Extents; i++){
                push_byte(bytes[i]);
            }
        }
    }

    constexpr __inline void on_overflow(){
        __builtin_trap();
    }

    constexpr __inline 
    void push_byte(const uint8_t byte){
        if(pos_ >= bytes_.size()) [[unlikely]] 
            on_overflow();
        bytes_[pos_++] = byte;
    }


    template<size_t Extents>
    constexpr __inline 
    void push_bytes(const std::span<const uint8_t, Extents> bytes){
        if(pos_ + bytes.size() > bytes_.size()) [[unlikely]]
            on_overflow();
        push_bytes_unchecked(bytes);
    }

};
}