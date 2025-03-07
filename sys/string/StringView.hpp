#pragma once

#include "sys/string/utils/StringUtils.hpp"
#include "sys/utils/hash_func.hpp"

#include <span>
#include <vector>
#include <string>
#include <string_view>
#include <optional>


namespace ymd{

class String;

class StringView {
public:

    // 构造函数
    StringView(const String & str);
    constexpr StringView(const ::std::string & str): data_(str.c_str()), size_(str.length()) {}
    constexpr StringView(const ::std::string_view & str): data_(str.data()), size_(str.length()) {}
    constexpr StringView(const char* str) : data_(str), size_(str ? strlen(str) : 0) {}
    constexpr StringView(const char* str, size_t size) : data_(str), size_(size) {}
    constexpr StringView(const std::nullopt_t): data_(nullptr), size_(0){;}

    constexpr StringView(const StringView & other): data_(other.data_), size_(other.size_){;}
    constexpr StringView(StringView && other): data_(other.data_), size_(other.size_){;}

    constexpr StringView& operator=(const StringView & other) {
        data_ = other.data_;
        size_ = other.size_;
        return *this;
    }

    constexpr StringView& operator=(StringView && other) {
        data_ = ::std::move(other.data_);
        size_ = ::std::move(other.size_);
        return *this;
    }

    constexpr bool operator==(const StringView & other) const { return size_ == other.size_ && memcmp(data_, other.data_, size_) == 0; }

    constexpr const char * begin() const {return data_;}
    constexpr const char * end() const {return data_ + size_;}

    constexpr size_t size() const { return size_; }
    constexpr size_t length() const {return size_;}

    constexpr bool empty() const { return size_ == 0; }

    constexpr const char* data() const { return data_; }

    template<integral T>
    constexpr explicit operator T() const {return StringUtils::atoi(this->data(), this->length());}

    template<floating T>
    constexpr explicit operator T() const {return StringUtils::atof(this->data(), this->length());}

    template<size_t Q>
    operator iq_t<Q>() const{return StringUtils::atoq<Q>(this->data_, this->size_);}
    constexpr char operator [](const size_t index) const {return data_[index];}

	std::vector<StringView> split(const char chr, const size_t max_pieces = 0) const;

	__fast_inline constexpr StringView substring(size_t left) const {return substring(left, size_ - left);};
	__fast_inline constexpr StringView substring(size_t left, size_t right) const {
        if (left > right) std::swap(left, right);
        
        if (left >= size_) return StringView(this->data_, 0);
        if (right > size_) right = size_;

        return StringView(this->data_ + left, right - left);
    }
    
	int indexOf(const char ch ) const;
	int indexOf(const char ch,const size_t fromIndex ) const;

    constexpr uint32_t hash() const {return hash_impl(data_, size_);}

private:
    const char * data_;
    size_t size_;
};

using StringViews = std::span<const StringView>;

}