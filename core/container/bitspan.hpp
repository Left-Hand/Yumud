#pragma once

#include <cstdint>
#include <span>
#include <iterator>

namespace ymd {

template<typename T, size_t N = std::dynamic_extent>
struct _BitSpan {
private:
    std::span<T> bytes_;
    
public:
    // Iterator class for bit iteration
    class iterator {
    private:
        const _BitSpan* parent_;
        size_t bit_pos_;
        
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = bool;
        using difference_type = std::ptrdiff_t;
        using pointer = bool*;
        using reference = bool;
        
        iterator(const _BitSpan* parent, size_t bit_pos) 
            : parent_(parent), bit_pos_(bit_pos) {}
            
        bool operator*() const {
            return parent_->test(bit_pos_);
        }
        
        iterator& operator++() {
            ++bit_pos_;
            return *this;
        }
        
        iterator operator++(int) {
            iterator tmp = *this;
            ++bit_pos_;
            return tmp;
        }
        
        iterator& operator--() {
            --bit_pos_;
            return *this;
        }
        
        iterator operator--(int) {
            iterator tmp = *this;
            --bit_pos_;
            return tmp;
        }
        
        iterator& operator+=(difference_type n) {
            bit_pos_ += n;
            return *this;
        }
        
        iterator& operator-=(difference_type n) {
            bit_pos_ -= n;
            return *this;
        }
        
        friend iterator operator+(iterator it, difference_type n) {
            return iterator(it.parent_, it.bit_pos_ + n);
        }
        
        friend iterator operator+(difference_type n, iterator it) {
            return it + n;
        }
        
        friend iterator operator-(iterator it, difference_type n) {
            return iterator(it.parent_, it.bit_pos_ - n);
        }
        
        friend difference_type operator-(const iterator& lhs, const iterator& rhs) {
            return lhs.bit_pos_ - rhs.bit_pos_;
        }
        
        bool operator==(const iterator& other) const {
            return parent_ == other.parent_ && bit_pos_ == other.bit_pos_;
        }
        
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
        
        bool operator<(const iterator& other) const {
            return bit_pos_ < other.bit_pos_;
        }
        
        bool operator>(const iterator& other) const {
            return bit_pos_ > other.bit_pos_;
        }
        
        bool operator<=(const iterator& other) const {
            return bit_pos_ <= other.bit_pos_;
        }
        
        bool operator>=(const iterator& other) const {
            return bit_pos_ >= other.bit_pos_;
        }
    };
    
    // Constructors
    _BitSpan() = default;
    
    explicit _BitSpan(std::span<T> bytes) : bytes_(bytes) {}
    
    // Capacity
    size_t size() const noexcept { return bytes_.size() * 8; }
    size_t size_bytes() const noexcept { return bytes_.size(); }
    bool empty() const noexcept { return bytes_.empty(); }
    
    // Element access
    bool test(size_t bit_pos) const {
        size_t byte_idx = bit_pos / 8;
        size_t bit_idx = bit_pos % 8;
        return (bytes_[byte_idx] >> bit_idx) & 0x01;
    }
    
    void set(size_t bit_pos, bool value = true) {
        if constexpr (!std::is_const_v<T>) {
            size_t byte_idx = bit_pos / 8;
            size_t bit_idx = bit_pos % 8;
            if (value) {
                bytes_[byte_idx] |= (1 << bit_idx);
            } else {
                bytes_[byte_idx] &= ~(1 << bit_idx);
            }
        }
    }
    

    struct MutableBitProxy{
        MutableBitProxy(T & byte, size_t bit_pos): byte_(byte), bit_pos_(bit_pos) {}
        operator bool() const { return byte_[bit_pos_ / 8] & (1 << (bit_pos_ % 8)); }
        MutableBitProxy & operator=(bool value) {
            // byte_ = (bit_pos_, value);
            const uint8_t mask = (1 << (bit_pos_ & 0b111));;
            if(value) bytes_ |= mask;
            else bytes_ &= ~mask;
            return *this;
        }
        MutableBitProxy & operator=(const MutableBitProxy & other)  = default;

    private:
        T & byte_;
        size_t bit_pos_;
    };
    bool operator[](size_t bit_pos) const { return test(bit_pos); }
    
    // Iterators
    iterator begin() const { return iterator(this, 0); }
    iterator end() const { return iterator(this, size()); }
    
    // Underlying storage access
    std::span<T> bytes() const { return bytes_; }

};

template<size_t N = std::dynamic_extent>
using BitSpan = _BitSpan<const uint8_t, N>;

template<size_t N = std::dynamic_extent>
using BitSpanMut = _BitSpan<uint8_t, N>;

} // namespace ymd