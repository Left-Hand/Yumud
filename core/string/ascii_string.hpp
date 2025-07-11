#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <algorithm>
#include <iterator>
#include <atomic>

namespace ymd {

class AsciiString {
public:
    // Iterator implementation
    class iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = char*;
        using reference = char&;

        constexpr iterator(pointer p) : ptr(p) {}
        constexpr reference operator*() const { return *ptr; }
        constexpr pointer operator->() { return ptr; }
        constexpr iterator& operator++() { ++ptr; return *this; }
        constexpr iterator operator++(int) { iterator tmp = *this; ++ptr; return tmp; }
        constexpr bool operator==(const iterator& other) const { return ptr == other.ptr; }
        constexpr bool operator!=(const iterator& other) const { return ptr != other.ptr; }
    private:
        char * ptr;
    };

    // Iterator implementation
    class const_iterator {

    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = const char*;
        using reference = const char &;

        constexpr const_iterator(pointer p) : ptr(p) {}
        constexpr reference operator*() const { return *ptr; }
        constexpr pointer operator->() { return ptr; }
        constexpr const_iterator& operator++() { ++ptr; return *this; }
        constexpr const_iterator operator++(int) { const_iterator tmp = *this; ++ptr; return tmp; }
        constexpr bool operator==(const const_iterator& other) const { return ptr == other.ptr; }
        constexpr bool operator!=(const const_iterator& other) const { return ptr != other.ptr; }
    private:
        const char* ptr;
    };

    // Constructors
    constexpr explicit AsciiString() : length_(0), is_sso_(true) {
        sso_buffer[0] = '\0';
    }

    explicit AsciiString(const char* str) {
        length_ = std::strlen(str);
        
        if (length_ <= SSO_CAPACITY) {
            is_sso_ = true;
            std::memcpy(sso_buffer, str, length_ + 1);
        } else {
            is_sso_ = false;
            dynamic_data = DynamicData::create(length_ + 1);
            std::memcpy(dynamic_data->data, str, length_ + 1);
        }
    }

    // Copy constructor (COW)
    AsciiString(const AsciiString& other) : length_(other.length_), is_sso_(other.is_sso_) {
        if (is_sso_) {
            std::memcpy(sso_buffer, other.sso_buffer, SSO_CAPACITY + 1);
        } else {
            dynamic_data = other.dynamic_data;
            ++dynamic_data->refcount;
        }
    }

    // Move constructor
    AsciiString(AsciiString&& other) noexcept {
        length_ = other.length_;
        is_sso_ = other.is_sso_;
        
        if (is_sso_) {
            std::memcpy(sso_buffer, other.sso_buffer, SSO_CAPACITY + 1);
        } else {
            dynamic_data = other.dynamic_data;
        }
        
        other.length_ = 0;
        other.is_sso_ = true;
        other.sso_buffer[0] = '\0';
    }

    ~AsciiString() {
        release();
    }

    // Assignment operators
    AsciiString& operator=(const AsciiString& other) {
        if (this != &other) {
            release();
            length_ = other.length_;
            is_sso_ = other.is_sso_;
            
            if (is_sso_) {
                std::memcpy(sso_buffer, other.sso_buffer, SSO_CAPACITY + 1);
            } else {
                dynamic_data = other.dynamic_data;
                ++dynamic_data->refcount;
            }
        }
        return *this;
    }

    AsciiString& operator=(AsciiString&& other) noexcept {
        if (this != &other) {
            release();
            length_ = other.length_;
            is_sso_ = other.is_sso_;
            
            if (is_sso_) {
                std::memcpy(sso_buffer, other.sso_buffer, SSO_CAPACITY + 1);
            } else {
                dynamic_data = other.dynamic_data;
            }
            
            other.length_ = 0;
            other.is_sso_ = true;
            other.sso_buffer[0] = '\0';
        }
        return *this;
    }

    // Example additions:
    constexpr bool operator==(const AsciiString& other) const {
        return length_ == other.length_ && 
            std::memcmp(c_str(), other.c_str(), length_) == 0;
    }

    constexpr const_iterator begin() const {
        return const_iterator{is_sso_ ? sso_buffer : dynamic_data->data};}
    constexpr const_iterator end() const {
        return const_iterator{(is_sso_ ? sso_buffer : dynamic_data->data) + length_};}

    constexpr size_t capacity() const { 
        return is_sso_ ? SSO_CAPACITY : dynamic_data->capacity - 1; 
    }

    // Access methods
    constexpr char& operator[](size_t index) {
        detach();
        return is_sso_ ? sso_buffer[index] : dynamic_data->data[index];
    }

    constexpr const char& operator[](size_t index) const {
        return is_sso_ ? sso_buffer[index] : dynamic_data->data[index];
    }

    constexpr const char* c_str() const {
        return is_sso_ ? sso_buffer : dynamic_data->data;
    }

    constexpr size_t size() const { return length_; }
    constexpr bool empty() const { return length_ == 0; }

    // Iterators
    iterator begin() { 
        detach();
        return iterator(is_sso_ ? sso_buffer : dynamic_data->data);
    }
    
    iterator end() { 
        detach();
        return iterator((is_sso_ ? sso_buffer : dynamic_data->data) + length_);
    }

private:
    static constexpr size_t SSO_CAPACITY = 15; // 15 chars + null terminator in SSO buffer

    struct DynamicData {
        std::atomic<size_t> refcount;
        size_t capacity;
        char data[1]; // Flexible array member

        static DynamicData* create(size_t size) {
            size_t alloc_size = sizeof(DynamicData) + size;
            auto* dd = static_cast<DynamicData*>(::operator new(alloc_size));
            new (dd) DynamicData{1, size};
            return dd;
        }
    };

    union {
        char sso_buffer[SSO_CAPACITY + 1]; // SSO storage (+1 for null terminator)
        DynamicData* dynamic_data;         // COW storage
    };

    size_t length_;
    bool is_sso_;

    // Helper methods
    void detach() {
        if (!is_sso_ && dynamic_data->refcount > 1) {
            DynamicData* new_data = DynamicData::create(dynamic_data->capacity);
            std::memcpy(new_data->data, dynamic_data->data, length_);
            release();
            dynamic_data = new_data;
        }
    }

    void release() {
        if (!is_sso_) {
            if (--dynamic_data->refcount == 0) {
                ::operator delete(dynamic_data);
            }
        }
    }
};

} // namespace ymd