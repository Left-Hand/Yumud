#pragma once

#include "steadywin_can_simple_primitive.hpp"
#include "core/utils/bytes/bytes_caster.hpp"

namespace ymd::robots::steadywin::can_simple{

struct [[nodiscard]] BytesFiller{
public:
    constexpr explicit BytesFiller(std::span<uint8_t> bytes):
        bytes_(bytes){;}

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


    constexpr __inline 
    void push_le_u8(const uint16_t int_val) {
        push_byte(int_val);
    }

    constexpr __inline 
    void push_le_u16(const uint16_t int_val) {
        push_byte(int_val & 0xFF);
        push_byte(int_val >> 8);
    }

    constexpr __inline
    void push_le_i16(const int16_t int_val) { 
        return push_le_u16(std::bit_cast<uint16_t>(int_val));
    }

    constexpr __inline 
    void push_le_u32(const uint32_t int_val){
        push_le_u16(int_val & 0xFFFF);
        push_le_u16(int_val >> 16);
    }

    constexpr __inline 
    void push_le_i32(const int32_t int_val){
        push_le_u16(int_val & 0xFFFF);
        push_le_u16(int_val >> 16);
    }

    constexpr 
    void push_fp32(const math::fp32 f_val){
        return push_le_u32(std::bit_cast<uint32_t>(f_val));
    }



    [[nodiscard]] constexpr bool is_full() const {
        return pos_ == bytes_.size();
    }

    size_t size() const {
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
};


struct [[nodiscard]] BytesReader{
    explicit constexpr BytesReader(std::span<const uint8_t> bytes) : 
        bytes_(bytes) {}

    [[nodiscard]] constexpr Option<int32_t> fetch_i32(){
        if(remaining().size() < 4)
            return None;
        return Some(le_bytes_to_int<int32_t>(fetch_bytes<4>()));
    }
    
    [[nodiscard]] constexpr Option<uint32_t> fetch_u32(){
        if(remaining().size() < 4)
            return None;
        return Some(le_bytes_to_int<uint32_t>(fetch_bytes<4>()));
    }


    [[nodiscard]] constexpr Option<math::fp32> fetch_f32(){
        if(remaining().size() < 4)
            return None;
        return Some(math::fp32::from_bits(le_bytes_to_int<int32_t>(fetch_bytes<4>())));
    }
private:
    std::span<const uint8_t> bytes_;

    template<size_t N>
    [[nodiscard]] constexpr std::span<const uint8_t, N> fetch_bytes(){
        const auto ret = std::span<const uint8_t, N>(bytes_.data(), N);
        bytes_ = std::span<const uint8_t>(bytes_.data() + N, bytes_.size() - N);
        return ret;
    }

    [[nodiscard]] constexpr std::span<const uint8_t> remaining() const {
        return bytes_;
    }
};

}