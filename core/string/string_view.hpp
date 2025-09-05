#pragma once

#include "core/string/utils/strconv.hpp"
#include "core/utils/hash_func.hpp"
#include "core/utils/option.hpp"
#include <span>
#include <vector>
#include <string>
#include <string_view>
#include <optional>


namespace ymd{

class String;

class StringView {
public:
    // 构造函数 从容器构造必须为显式 避免调用者没注意到生命周期
    constexpr explicit StringView(const std::string & str): data_(str.c_str()), size_(str.length()) {}

    constexpr StringView(const std::string_view str): data_(str.data()), size_(str.length()) {}
    constexpr StringView(const char* str) : data_(str), size_(str ? strlen(str) : 0) {}
    constexpr StringView(const char* str, size_t size) : data_(str), size_(size) {}
    constexpr StringView(const StringView & other): data_(other.data_), size_(other.size_){;}
    constexpr StringView(StringView && other): data_(other.data_), size_(other.size_){;}

    constexpr StringView& operator=(const StringView & other) {
        data_ = other.data_;
        size_ = other.size_;
        return *this;
    }

    constexpr StringView& operator=(StringView && other) {
        data_ = std::move(other.data_);
        size_ = std::move(other.size_);
        return *this;
    }

    constexpr bool operator==(const StringView & other) const { 

        if(size_ != other.size_) return false;

        #pragma GCC unroll 4
        for(size_t i = 0; i < size_; i++){
            if(data_[i] != other.data_[i]) return false;
        }

        return true;
    }

    constexpr const char * begin() const {return data_;}
    constexpr const char * c_str() const {return data_;}
    constexpr const char * end() const {return data_ + size_;}

    constexpr size_t size() const { return size_; }
    constexpr size_t length() const {return size_;}

    constexpr bool is_empty() const { return size_ == 0; }

    constexpr const char* data() const { return data_; }

    constexpr char operator [](const size_t index) const {return data_[index];}
	__fast_inline constexpr StringView substr(size_t left) const {
        return substr_by_range(left, size_);};
	__fast_inline constexpr StringView substr_by_range(size_t left, size_t right) const {
        if (unlikely(left > right)) std::swap(left, right);
        
        if (left >= size_) 
            return StringView(this->data_, 0);

        if (right > size_) right = size_;

        return StringView(this->data_ + left, right - left);
    }

	__fast_inline constexpr StringView substr_by_len(size_t left, size_t len) const {
        return substr_by_range(left, left + len);
    }
    
    constexpr Option<size_t> find(char c) const{
        return find_from(c, 0);
    }

    constexpr Option<size_t> find_from(char ch, size_t from) const{
        if (from >= size_) return None;
        for(size_t i = from; i < size_; i++){
            if(data_[i] == ch) return Some(i);
        }
        return None;
    }


    constexpr uint32_t hash() const {return ymd::hash(*this);}

    constexpr StringView trim() const {
        auto & self = *this;
        auto is_whitespace = [](char c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        };

        if (self.is_empty()) {
            return self;
        }

        // Find first non-whitespace character
        size_t start = 0;
        while (start < self.size() && is_whitespace(self[start])) {
            ++start;
        }

        // Find last non-whitespace character
        size_t end = self.size();
        while (end > start && is_whitespace(self[end - 1])) {
            --end;
        }

        return self.substr_by_len(start, end - start);
    }

    constexpr operator std::string_view() const {
        return std::string_view(data_, size_);
    }

    std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(data_), size_);
    }
private:
    const char * data_;
    size_t size_;
};


}