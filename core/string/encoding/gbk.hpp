#pragma once

#include "core/string/view/string_view.hpp"

namespace ymd{

struct alignas(4) [[nodiscard]] GbkChar final{

    wchar_t bits;

    constexpr explicit GbkChar(std::span<const uint8_t, 3> _bytes):
        bits{static_cast<wchar_t>(_bytes[0] << 8 | _bytes[1])}{;}

    constexpr explicit GbkChar(const wchar_t ch):
        bits{ch}{;}

    static constexpr GbkChar zero(){
        return GbkChar{0};
    }

    constexpr wchar_t count() const {
        return bits;
    }

    constexpr bool operator ==(const GbkChar & rhs) const{
        return bits == rhs.bits;
    }
};

class [[nodiscard]] GbkIterator final {
public:
    constexpr explicit GbkIterator(const StringView str) : 
        str_(str), 
        idx_(0) {}
    
    [[nodiscard]] constexpr bool has_next() const {
        return idx_ < str_.size() && str_[idx_] != '\0';
    }
    
    [[nodiscard]] constexpr GbkChar next() {
        if (!has_next()) {
            return GbkChar::zero();
        }
        
        unsigned char first_byte = static_cast<unsigned char>(str_[idx_]);
        unsigned int gbk_value = 0;
        
        if (first_byte < 0x80) { // ASCII字符
            gbk_value = first_byte;
            ++idx_;
        } 
        else { // GBK双字节字符
            if (idx_ + 1 >= str_.size()) {
                // 处理不完整的GBK字符
                gbk_value = first_byte;
                ++idx_;
            } else {
                unsigned char second_byte = static_cast<unsigned char>(str_[idx_ + 1]);
                
                // 组合成GBK编码值
                gbk_value = (static_cast<unsigned int>(first_byte) << 8) | second_byte;
                idx_ += 2;
            }
        }
        
        return GbkChar(gbk_value);
    }
    
    constexpr size_t current_index() const {
        return idx_;
    }

private:
    StringView str_;
    size_t idx_;
};

}