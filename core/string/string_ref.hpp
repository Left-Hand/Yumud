#pragma once

#include <cstdint>
#include <cstring>
#include <string_view>
#include <algorithm>

#include "core/utils/Option.hpp"

namespace ymd{

struct [[nodiscard]] StringRef{
    constexpr StringRef(const StringRef & other) noexcept: 
        data_(other.data_), size_(other.size_){;}
    constexpr StringRef(StringRef && other) noexcept: 
        data_(other.data_), size_(other.size_) {}
    constexpr explicit StringRef(char* str, size_t size) noexcept: 
        data_(str), size_(size) {}

    template<size_t N>
    constexpr explicit StringRef(char (&str)[N]) noexcept: 
        data_(str), size_(N){;}

    constexpr StringRef & operator =(const StringRef & other) noexcept = default;
    constexpr StringRef & operator =(StringRef && other) noexcept = default;

    [[nodiscard]] constexpr const char * begin() const noexcept {return data_;}
    [[nodiscard]] constexpr char * begin() noexcept {return data_;} 
    [[nodiscard]] constexpr const char * end() const noexcept {return data_ + size_;}
    [[nodiscard]] constexpr char * end() noexcept {return data_ + size_;}

    [[nodiscard]] constexpr size_t size() const noexcept {return size_;}

    [[nodiscard]] constexpr size_t length() const noexcept {return size_;}

    [[nodiscard]] constexpr char& operator[](size_t pos) noexcept { return data_[pos]; }
    [[nodiscard]] constexpr const char& operator[](size_t pos) const noexcept { return data_[pos]; }
    
    [[nodiscard]] constexpr char& front() noexcept { return *data_; }
    [[nodiscard]] constexpr const char& front() const noexcept { return *data_; }
    
    [[nodiscard]] constexpr char& back() noexcept { return data_[size_ - 1]; }
    [[nodiscard]] constexpr const char& back() const noexcept { return data_[size_ - 1]; }
    
    [[nodiscard]] constexpr char* data() noexcept { return data_; }
    [[nodiscard]] constexpr const char* data() const noexcept { return data_; }

    // String operations
    [[nodiscard]] constexpr Option<StringRef> substr(size_t pos, size_t count) const noexcept {
        // 检查pos是否超出范围
        if (pos > size_) {
            return None;
        }
        
        // 计算安全的count值
        const size_t available = size_ - pos;
        const size_t safe_count = std::min(count, available);
        
        return Some(StringRef(data_ + pos, safe_count));
    }

    [[nodiscard]] constexpr Option<StringRef> substr(size_t pos) const noexcept {
        // 检查pos是否超出范围
        if (pos > size_) {
            return None;
        }
        
        return Some(StringRef(data_ + pos, size_ - pos));
    }

    // String operations
    [[nodiscard]] constexpr StringRef substr_unchecked(size_t pos, size_t count) const noexcept {
        return StringRef(data_ + pos, std::min(count, size_ - pos));
    }

    [[nodiscard]] constexpr StringRef substr_unchecked(size_t pos) const noexcept {
        return StringRef(data_ + pos, size_ - pos);
    }
private:
    char * data_;
    size_t size_;
};


}