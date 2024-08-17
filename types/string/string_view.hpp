#pragma once

#include "sys/core/platform.h"
#include "sys/kernel/string_utils.hpp"

#include <cstdint>
#include <memory.h>
#include <string.h>

#include <string>
#include <string_view>

class String;

class StringView {
public:
    // 构造函数
    StringView(const String & str);
    StringView(const std::string & str): data_(str.c_str()), size_(str.length()) {}
    StringView(const std::string_view & str): data_(str.data()), size_(str.length()) {}
    StringView(const char* str) : data_(str), size_(str ? strlen(str) : 0) {}
    StringView(const char* str, size_t size) : data_(str), size_(size) {}

    StringView(const StringView & other): data_(other.data_), size_(other.size_){;}
    StringView(StringView && other): data_(other.data_), size_(other.size_){;}
    StringView& operator=(const StringView& other) {
        data_ = other.data_;
        size_ = other.size_;
        return *this;
    }

    size_t size() const { return size_; }
    size_t length() const {return size_;}

    bool empty() const { return size_ == 0; }

    const char* data() const { return data_; }

    template<integral T>
    operator T() const {return StringUtils::stoi(this->data());}

    operator float() const {return StringUtils::stof(this->data());}

    operator double() const {return float(*this);}
private:
    const char* data_;
    size_t size_;
};