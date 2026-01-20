#pragma once

#include "core/utils/hash_func.hpp"
#include "core/utils/option.hpp"

namespace ymd{

class String;

template<size_t Extents>
struct [[nodiscard]] ExtentsStorage final {
    static constexpr size_t MAX_SIZE = Extents;
    static consteval size_t length() const { return MAX_SIZE; }
};

template<std::size_t Extents = std::dynamic_extent>
struct [[nodiscard]] ExtentsStorage final { 
    size_t length_;
    constexpr size_t length() const { return length_; }
};

template<typename Encoding, size_t Extents = std::dynamic_extent>
class [[nodiscard]] StringSpan {
public:
    // 构造函数 从容器构造必须为显式 避免调用者没注意到生命周期
    constexpr explicit StringSpan(const std::string & str) noexcept: 
        p_data_(str.data()), size_(str.length()) {}
    constexpr StringSpan(const std::string_view str) noexcept: 
        p_data_(str.data()), size_(str.length()) {}


    //从不原始c字符串构造考虑到不带尾0的攻击 只能在编译期使用 运行期使用from_cstr
    consteval StringSpan(const Encoding * p_chars) noexcept: 
        p_data_(p_chars), size_((p_chars != nullptr) ? strlen(p_chars) : 0) {}

    constexpr StringSpan(const Encoding * p_chars, size_t size) noexcept: 
        p_data_(p_chars), size_(size) {}

    static constexpr StringSpan from_cstr(const Encoding * p_chars, size_t max_size = std::dynamic_extent) noexcept{
        return from_zero_terminated(p_chars, max_size);
    }
    
    static constexpr StringSpan from_zero_terminated(const Encoding * p_chars, size_t max_size = std::dynamic_extent) noexcept{
        const size_t size = (p_chars != nullptr) ? strnlen(p_chars, max_size) : 0;
        return StringSpan(p_chars, size);
    }

    template<size_t N>
    constexpr StringSpan(const Encoding (&str)[N]) noexcept:
        p_data_(str), size_(strnlen(str, N)){}
    constexpr StringSpan(const StringSpan & other) noexcept: 
        p_data_(other.p_data_), size_(other.size_){;}
    constexpr StringSpan(StringSpan && other) noexcept: 
        p_data_(other.p_data_), size_(other.size_){;}

    constexpr StringSpan& operator=(const StringSpan & other) noexcept{
        p_data_ = other.p_data_;
        size_ = other.size_;
        return *this;
    }

    constexpr StringSpan& operator=(StringSpan && other) noexcept{
        p_data_ = std::move(other.p_data_);
        size_ = std::move(other.size_);
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const StringSpan & other) const noexcept{ 

        if(size_ != other.size_) return false;

        #pragma GCC unroll 4
        for(size_t i = 0; i < size_; i++){
            if(p_data_[i] != other.p_data_[i]) return false;
        }

        return true;
    }

    [[nodiscard]] constexpr const Encoding * begin() const noexcept{return p_data_;}
    [[nodiscard]] constexpr const Encoding * end() const noexcept{return p_data_ + size_;}

    [[nodiscard]] constexpr size_t size() const noexcept{ return size_; }
    [[nodiscard]] constexpr size_t length() const noexcept{return size_;}

    [[nodiscard]] constexpr const Encoding* data() const noexcept{ return p_data_; }

    [[nodiscard]] constexpr Encoding operator [](const size_t index) const noexcept{return p_data_[index];}
	__fast_inline constexpr Option<StringSpan> substr(size_t left) const noexcept{
        return substr_by_range(left, size_);};
	__fast_inline constexpr Option<StringSpan> substr_by_range(size_t left, size_t right) const noexcept{
        if (unlikely(left > right)) 
            return None;
        
        if (right > size_) 
            return None;

        return Some(StringSpan(this->p_data_ + left, right - left));
    }

	__fast_inline constexpr Option<StringSpan> substr_by_len(size_t left, size_t len) const noexcept{
        if (left + len > size_) 
            return None;

        return Some(StringSpan(this->p_data_ + left, len));
    }
    
    [[nodiscard]] constexpr Option<size_t> find(Encoding c) const noexcept{
        return find_from(c, 0);
    }

    [[nodiscard]] constexpr Option<size_t> find_from(Encoding ch, size_t from) const noexcept{
        if (from >= size_) return None;
        for(size_t i = from; i < size_; i++){
            if(p_data_[i] == ch) return Some(i);
        }
        return None;
    }


    [[nodiscard]] constexpr uint32_t hash() const noexcept{return ymd::hash(*this);}

    [[nodiscard]] constexpr StringSpan trim() const noexcept{
        auto & self = *this;
        auto is_whitespace = [](Encoding c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        };

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

        return StringSpan(
            self.data() + start, 
            static_cast<size_t>(MAX(static_cast<int>(end - start), 0))
        );
    }

    [[nodiscard]] constexpr operator std::string_view() const noexcept{
        return std::string_view(p_data_, size_);
    }

    [[nodiscard]] std::span<const uint8_t> as_bytes() const noexcept{
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(p_data_), size_);
    }

    [[nodiscard]] std::span<const unsigned Encoding> as_uchars() const noexcept{
        return std::span<const unsigned Encoding>(reinterpret_cast<const unsigned Encoding *>(p_data_), size_);
    }

    [[nodiscard]] constexpr std::span<const Encoding> chars() const noexcept{
        return std::span<const Encoding>(p_data_, size_);
    }
private:
    Encoding * p_data_;
    size_t size_;
};


}