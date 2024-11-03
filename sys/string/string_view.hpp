#pragma once

#include "sys/string/string_utils.hpp"
#include "algo/hash_func.hpp"

#include <cstdint>
#include <memory.h>
#include <string.h>

#include <vector>
#include <string>
#include <string_view>


namespace yumud{

class String;

class StringView {
public:
    using Strings = ::std::vector<StringView>;

    // 构造函数
    StringView(const String & str);
    StringView(const ::std::string & str): data_(str.c_str()), size_(str.length()) {}
    StringView(const ::std::string_view & str): data_(str.data()), size_(str.length()) {}
    StringView(const char* str) : data_(str), size_(str ? strlen(str) : 0) {}
    StringView(const char* str, size_t size) : data_(str), size_(size) {}

    StringView(const StringView & other): data_(other.data_), size_(other.size_){;}
    StringView(StringView && other): data_(other.data_), size_(other.size_){;}

    StringView& operator=(const StringView & other) {
        data_ = other.data_;
        size_ = other.size_;
        return *this;
    }

    StringView& operator=(StringView && other) {
        data_ = ::std::move(other.data_);
        size_ = ::std::move(other.size_);
        return *this;
    }

    size_t size() const { return size_; }
    size_t length() const {return size_;}

    bool empty() const { return size_ == 0; }

    const char* data() const { return data_; }

    template<integral T>
    explicit operator T() const {return StringUtils::atoi(this->data(), this->length());}

    template<floating T>
    explicit operator T() const {return StringUtils::atof(this->data(), this->length());}

    operator iq_t() const;
    char operator [](const size_t index) const {return data_[index];}

	Strings split(const char chr, const size_t times = 0) const;

	StringView substring( size_t beginIndex ) const { return substring(beginIndex, size_ - beginIndex);};
	StringView substring( size_t beginIndex, size_t endIndex ) const;
    
	int indexOf( char ch ) const;
	int indexOf( char ch, size_t fromIndex ) const;

    uint32_t hash() const {return hash_impl(data_, size_);}
    
	// int indexOf( const String &str ) const;
	// int indexOf( const String &str, size_t fromIndex ) const;
	// int lastIndexOf( char ch ) const;
	// int lastIndexOf( char ch, size_t fromIndex ) const;
	// int lastIndexOf( const String &str ) const;
	// int lastIndexOf( const String &str, size_t fromIndex ) const;
private:
    const char * data_;
    size_t size_;
};

}