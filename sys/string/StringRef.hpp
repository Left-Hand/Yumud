#pragma once

#include "sys/core/platform.h"
#include "sys/string/utils/StringUtils.hpp"

#include <cstdint>
#include <memory.h>
#include <string.h>

#include <string>
#include <string_view>

namespace ymd{

class StringRef {
public:
    // 构造函数
    StringRef(std::string & str): data_(str.data()), size_(str.length()) {}
    StringRef(char* str) : data_(str), size_(str ? strlen(str) : 0) {}
    StringRef(char* str, size_t size) : data_(str), size_(size) {}

    StringRef(const StringRef & other): data_(other.data_), size_(other.size_){;}
    StringRef(StringRef && other): data_(other.data_), size_(other.size_){;}
    StringRef& operator=(const StringRef& other) {
        data_ = other.data_;
        size_ = other.size_;
        return *this;
    }

    size_t size() const { return size_; }
    size_t length() const {return size_;}

    bool empty() const { return size_ == 0; }

    const char* data() const { return data_; }
    char* data() { return data_; }

private:
    char * data_;
    size_t size_;
};

}