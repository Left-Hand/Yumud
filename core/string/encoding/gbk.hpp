#pragma once

#include "core/string/string_view.hpp"
namespace ymd{

class GBKIterator final {
public:
    constexpr explicit GBKIterator(const StringView str) : 
        str_(str), 
        current_index_(0) {}
    
    constexpr bool has_next() const {
        return current_index_ < str_.size() && str_[current_index_] != '\0';
    }
    
    constexpr unsigned int next() {
        if (!has_next()) {
            return 0;
        }
        
        unsigned char first_byte = static_cast<unsigned char>(str_[current_index_]);
        unsigned int gbk_value = 0;
        
        if (first_byte < 0x80) { // ASCII字符
            gbk_value = first_byte;
            ++current_index_;
        } 
        else { // GBK双字节字符
            if (current_index_ + 1 >= str_.size()) {
                // 处理不完整的GBK字符
                gbk_value = first_byte;
                ++current_index_;
            } else {
                unsigned char second_byte = static_cast<unsigned char>(str_[current_index_ + 1]);
                
                // 组合成GBK编码值
                gbk_value = (static_cast<unsigned int>(first_byte) << 8) | second_byte;
                current_index_ += 2;
            }
        }
        
        return gbk_value;
    }
    
    constexpr size_t current_index() const {
        return current_index_;
    }

private:
    StringView str_;
    size_t current_index_;
};

}