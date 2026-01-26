#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <algorithm>
#include <iterator>
#include <atomic>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "core/utils/option.hpp"

namespace ymd::str {


// UTF-8编码验证和工具函数
namespace utf8 {
    constexpr bool is_continuation_byte(uint8_t byte) noexcept {
        return (byte & 0xC0) == 0x80;
    }

    constexpr Option<size_t> code_point_length(uint8_t first_byte) noexcept {
        if ((first_byte & 0x80) == 0x00) return Option<size_t>(1); // 1-byte sequence
        if ((first_byte & 0xE0) == 0xC0) return Option<size_t>(2); // 2-byte sequence
        if ((first_byte & 0xF0) == 0xE0) return Option<size_t>(3); // 3-byte sequence
        if ((first_byte & 0xF8) == 0xF0) return Option<size_t>(4); // 4-byte sequence
        return Option<size_t>{}; // Invalid first byte
    }

    constexpr bool validate_byte_sequence(const char* data, size_t length, size_t pos) {
        if (pos >= length) return false;
        
        auto len_opt = code_point_length(static_cast<uint8_t>(data[pos]));
        if (!len_opt.is_some()) return false;
        
        size_t expected_len = len_opt.unwrap();
        if (pos + expected_len > length) return false;
        
        for (size_t i = 1; i < expected_len; ++i) {
            if (!is_continuation_byte(static_cast<uint8_t>(data[pos + i]))) {
                return false;
            }
        }
        return true;
    }

    constexpr bool is_valid_utf8(const char* data, size_t length) noexcept {
        size_t pos = 0;
        while (pos < length) {
            auto len_opt = code_point_length(static_cast<uint8_t>(data[pos]));
            if (!len_opt.is_some()) return false;
            
            size_t expected_len = len_opt.unwrap();
            if (pos + expected_len > length) return false;
            
            for (size_t i = 1; i < expected_len; ++i) {
                if (!is_continuation_byte(static_cast<uint8_t>(data[pos + i]))) {
                    return false;
                }
            }
            pos += expected_len;
        }
        return pos == length;
    }
}

class Utf8String {
public:
    // UTF-8字符视图（代码点）
    class CodePoint {
    public:
        constexpr CodePoint() noexcept : data_(nullptr), length_(0) {}
        constexpr CodePoint(const char* data, size_t length) noexcept 
            : data_(data), length_(length) {}
        
        constexpr const char* data() const noexcept { return data_; }
        constexpr size_t length() const noexcept { return length_; }
        constexpr std::string_view view() const noexcept { 
            return std::string_view(data_, length_); 
        }
        
        constexpr bool operator==(const CodePoint& other) const noexcept {
            return length_ == other.length_ && 
                   std::memcmp(data_, other.data_, length_) == 0;
        }
        constexpr bool operator!=(const CodePoint& other) const noexcept {
            return !(*this == other);
        }
        
    private:
        const char* data_;
        size_t length_;
    };

    // 迭代器实现
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = CodePoint;
        using difference_type = std::ptrdiff_t;
        using pointer = CodePoint*;
        using reference = CodePoint&;

        constexpr iterator(const Utf8String* str, size_t pos) 
            : parent_(str), current_pos_(pos) {}
        
        constexpr CodePoint operator*() const {
            return parent_->code_point_at(current_pos_);
        }
        
        constexpr iterator& operator++() {
            current_pos_ = parent_->next_code_point_position(current_pos_);
            return *this;
        }
        
        constexpr iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        constexpr bool operator==(const iterator& other) const {
            return parent_ == other.parent_ && current_pos_ == other.current_pos_;
        }
        
        constexpr bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
        
    private:
        const Utf8String* parent_;
        size_t current_pos_;
    };

    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = CodePoint;
        using difference_type = std::ptrdiff_t;
        using pointer = const CodePoint*;
        using reference = const CodePoint&;

        constexpr const_iterator(const Utf8String* str, size_t pos) 
            : parent_(str), current_pos_(pos) {}
        
        constexpr CodePoint operator*() const {
            return parent_->code_point_at(current_pos_);
        }
        
        constexpr const_iterator& operator++() {
            current_pos_ = parent_->next_code_point_position(current_pos_);
            return *this;
        }
        
        constexpr const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        constexpr bool operator==(const const_iterator& other) const {
            return parent_ == other.parent_ && current_pos_ == other.current_pos_;
        }
        
        constexpr bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }
        
    private:
        const Utf8String* parent_;
        size_t current_pos_;
    };

    // 构造函数
    constexpr explicit Utf8String() noexcept : ascii_str_() {}
    
    static constexpr Option<Utf8String> from_str(std::string_view sv) noexcept {
        if (!utf8::is_valid_utf8(sv.data(), sv.size())) {
            return Option<Utf8String>{};
        }
        return Option<Utf8String>{Utf8String(sv)};
    }

    
    // 拷贝构造函数
    constexpr Utf8String(const Utf8String& other) = default;
    
    // 移动构造函数
    constexpr Utf8String(Utf8String&& other) noexcept = default;
    
    ~Utf8String() = default;

    // 赋值运算符
    constexpr Utf8String& operator=(const Utf8String& other) = default;
    constexpr Utf8String& operator=(Utf8String&& other) noexcept = default;

    // 访问方法
    constexpr const char* data() const noexcept { return ascii_str_.c_str(); }
    constexpr size_t size() const noexcept { return ascii_str_.size(); }
    constexpr size_t length() const noexcept { return ascii_str_.size(); }
    constexpr bool empty() const noexcept { return ascii_str_.empty(); }
    constexpr const char* c_str() const noexcept { return ascii_str_.c_str(); }
    
    constexpr std::string_view view() const noexcept {
        return std::string_view(ascii_str_.c_str(), ascii_str_.size());
    }

    // 代码点访问
    constexpr Option<CodePoint> code_point_at(size_t pos) const noexcept {
        if (pos >= ascii_str_.size()) return Option<CodePoint>{};
        
        auto len_opt = utf8::code_point_length(
            static_cast<uint8_t>(ascii_str_.c_str()[pos]));
        if (!len_opt.is_some()) return Option<CodePoint>{};
        
        size_t len = len_opt.unwrap();
        if (pos + len > ascii_str_.size()) return Option<CodePoint>{};
        
        return Option<CodePoint>{CodePoint(ascii_str_.c_str() + pos, len)};
    }
    
    constexpr size_t next_code_point_position(size_t pos) const noexcept {
        if (pos >= ascii_str_.size()) return ascii_str_.size();
        
        auto len_opt = utf8::code_point_length(
            static_cast<uint8_t>(ascii_str_.c_str()[pos]));
        if (!len_opt.is_some()) return ascii_str_.size();
        
        return pos + len_opt.unwrap();
    }
    
    constexpr size_t code_point_count() const noexcept {
        size_t count = 0;
        size_t pos = 0;
        while (pos < ascii_str_.size()) {
            auto len_opt = utf8::code_point_length(
                static_cast<uint8_t>(ascii_str_.c_str()[pos]));
            if (!len_opt.is_some()) break;
            
            pos += len_opt.unwrap();
            ++count;
        }
        return count;
    }

    // 迭代器
    constexpr iterator begin() { return iterator(this, 0); }
    constexpr iterator end() { return iterator(this, ascii_str_.size()); }
    constexpr const_iterator begin() const { return const_iterator(this, 0); }
    constexpr const_iterator end() const { return const_iterator(this, ascii_str_.size()); }
    constexpr const_iterator cbegin() const { return const_iterator(this, 0); }
    constexpr const_iterator cend() const { return const_iterator(this, ascii_str_.size()); }

    // 比较操作
    constexpr bool operator==(const Utf8String& other) const noexcept {
        return ascii_str_ == other.ascii_str_;
    }
    
    constexpr bool operator!=(const Utf8String& other) const noexcept {
        return !(*this == other);
    }
    
    constexpr bool operator<(const Utf8String& other) const noexcept {
        return std::strcmp(ascii_str_.c_str(), other.ascii_str_.c_str()) < 0;
    }
    
    constexpr bool operator>(const Utf8String& other) const noexcept {
        return other < *this;
    }
    
    constexpr bool operator<=(const Utf8String& other) const noexcept {
        return !(other < *this);
    }
    
    constexpr bool operator>=(const Utf8String& other) const noexcept {
        return !(*this < other);
    }

    // 子字符串操作（返回Option确保安全）
    constexpr Option<Utf8String> substr(size_t pos, size_t count = std::string_view::npos) const noexcept {
        if (pos > ascii_str_.size()) return Option<Utf8String>{};
        
        size_t actual_count = std::min(count, ascii_str_.size() - pos);
        std::string_view sv(ascii_str_.c_str() + pos, actual_count);
        
        // 确保子字符串从有效的UTF-8边界开始
        if (pos > 0 && utf8::is_continuation_byte(
            static_cast<uint8_t>(ascii_str_.c_str()[pos]))) {
            return Option<Utf8String>{};
        }
        
        // 验证子字符串是有效的UTF-8
        if (!utf8::is_valid_utf8(sv.data(), sv.size())) {
            return Option<Utf8String>{};
        }
        
        return Option<Utf8String>{Utf8String(sv)};
    }

    // 查找操作
    constexpr Option<size_t> find(char c, size_t pos = 0) const noexcept {
        for (size_t i = pos; i < ascii_str_.size(); ++i) {
            if (ascii_str_.c_str()[i] == c) {
                return Option<size_t>{i};
            }
        }
        return Option<size_t>{};
    }
    
    constexpr Option<size_t> find(std::string_view sv, size_t pos = 0) const noexcept {
        if (sv.empty() || pos > ascii_str_.size()) return Option<size_t>{};
        
        for (size_t i = pos; i <= ascii_str_.size() - sv.size(); ++i) {
            if (std::memcmp(ascii_str_.c_str() + i, sv.data(), sv.size()) == 0) {
                return Option<size_t>{i};
            }
        }
        return Option<size_t>{};
    }

private:
    AsciiString ascii_str_;
};

// 工具函数
constexpr bool is_valid_utf8(const char* str) noexcept {
    return utf8::is_valid_utf8(str, std::strlen(str));
}

constexpr bool is_valid_utf8(std::string_view sv) noexcept {
    return utf8::is_valid_utf8(sv.data(), sv.size());
}

} // namespace ymd