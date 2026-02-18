#pragma once

#include "core/utils/hash_func.hpp"
#include "core/utils/option.hpp"

namespace ymd{

class String;
class StringView;

struct alignas(sizeof(size_t) * 2) [[nodiscard]] StringView {
public:
    // 构造函数 从容器构造必须为显式 避免调用者没注意到生命周期
    constexpr explicit StringView(const std::string & str) noexcept: 
        p_str_(str.data()), length_(str.length()) {}
    constexpr StringView(const std::string_view str) noexcept: 
        p_str_(str.data()), length_(str.length()) {}


    //从不原始c字符串构造考虑到不带尾0的攻击 只能在编译期使用 运行期使用from_cstr
    consteval StringView(const char * p_chars) noexcept: 
        p_str_(p_chars), length_((p_chars != nullptr) ? strlen(p_chars) : 0) {}

    constexpr StringView(const char * p_chars, size_t size) noexcept: 
        p_str_(p_chars), length_(size) {}

    static constexpr StringView from_cstr(
        const char * p_chars, 
        size_t max_size = std::dynamic_extent
    ) noexcept{
        return from_zero_terminated(p_chars, max_size);
    }
    
    static constexpr StringView from_zero_terminated(
        const char * p_chars, 
        size_t max_size = std::dynamic_extent
    ) noexcept{
        const size_t size = (p_chars != nullptr) ? strnlen(p_chars, max_size) : 0;
        return StringView(p_chars, size);
    }

    template<size_t N>
    constexpr StringView(const char (&str)[N]) noexcept:
        p_str_(str), length_(strnlen(str, N)){}
    constexpr StringView(const StringView & other) noexcept: 
        p_str_(other.p_str_), length_(other.length_){;}
    constexpr StringView(StringView && other) noexcept: 
        p_str_(other.p_str_), length_(other.length_){;}

    constexpr StringView& operator=(const StringView & other) noexcept{
        p_str_ = other.p_str_;
        length_ = other.length_;
        return *this;
    }

    constexpr StringView& operator=(StringView && other) noexcept{
        p_str_ = std::move(other.p_str_);
        length_ = std::move(other.length_);
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const StringView & other) const noexcept{ 

        if(length_ != other.length_) return false;

        #pragma GCC unroll 4
        for(size_t i = 0; i < length_; i++){
            if(p_str_[i] != other.p_str_[i]) return false;
        }

        return true;
    }

    [[nodiscard]] constexpr bool equals(const StringView other) const noexcept{ 
        return *this == other;
    }

    [[nodiscard]] constexpr const char * begin() const noexcept{return p_str_;}
    [[nodiscard]] constexpr const char * end() const noexcept{return p_str_ + length_;}

    [[nodiscard]] constexpr size_t size() const noexcept{ return length_; }
    [[nodiscard]] constexpr size_t length() const noexcept{return length_;}

    [[nodiscard]] constexpr bool is_empty() const noexcept{return length_ == 0;}

    [[nodiscard]] constexpr const char* data() const noexcept{ return p_str_; }

    [[nodiscard]] constexpr char operator [](const size_t index) const noexcept{return p_str_[index];}
	__fast_inline constexpr Option<StringView> substr(size_t left) const noexcept{
        return substr_by_range(left, length_);};
	
    __fast_inline constexpr Option<StringView> 
    substr_by_range(size_t left, size_t right) const noexcept{
        if ((left > right)) [[unlikely]] 
            return None;
        
        if (right > length_) 
            return None;

        return Some(StringView(this->p_str_ + left, right - left));
    }

	__fast_inline constexpr Option<StringView> 
    substr_by_len(size_t left, size_t len) const noexcept{
        if (left + len > length_) 
            return None;

        return Some(StringView(this->p_str_ + left, len));
    }
    
    [[nodiscard]] constexpr Option<size_t> find(char c) const noexcept{
        return find_from_fn(0, [=](const char chr) -> bool {return c == chr;});
    }

    [[nodiscard]] constexpr Option<size_t> rfind(char c) const noexcept{
        return rfind_from_fn(0, [=](const char chr) -> bool {return c == chr;});
    }

    template<typename Fn>
    [[nodiscard]] constexpr Option<size_t> find_from_fn(size_t from, Fn && fn) const noexcept{
        if (from >= length_) return None;
        for(size_t i = from; i < length_; i++){
            if(std::forward<Fn>(fn)(p_str_[i])) return Some(i);
        }
        return None;
    }

    template<typename Fn>
    [[nodiscard]] constexpr Option<size_t> rfind_from_fn(size_t from, Fn && fn) const noexcept{
        if (from >= length_) return None;
        
        for(int i = from-1; i >= 0; i--){
            if(std::forward<Fn>(fn)(p_str_[i])) return Some(i);
        }
        return None;
    }


    [[nodiscard]] constexpr uint32_t hash() const noexcept{return ymd::hash(*this);}

    [[nodiscard]] constexpr std::tuple<StringView, StringView> 
    split_at(const size_t mid) const noexcept{
        return std::make_tuple(StringView(p_str_, mid), StringView(p_str_ + mid, length_ - mid));
    }

    template<typename Fn>
    [[nodiscard]] constexpr StringView trim_withfn(Fn && fn) const noexcept{
        auto & self = *this;

        // Handle null data case
        if (self.p_str_ == nullptr || self.length_ == 0) {
            return StringView(nullptr, 0);
        }

        // Find first non-whitespace character
        size_t start = 0;
        while (start < self.length_ && std::forward<Fn>(fn)(self[start])) {
            ++start;
            
            // Prevent infinite loop in case all characters match predicate
            if (start >= self.length_) {
                return StringView(self.p_str_ + start, 0);
            }
        }

        // Find last non-whitespace character
        size_t stop = self.length_;
        while (stop > start && std::forward<Fn>(fn)(self[stop - 1])) {
            --stop;
        }

        // Ensure start and stop are within bounds
        if (stop <= start || start >= self.length_ || stop > self.length_) {
            return StringView(self.p_str_ + start, 0);
        }

        return StringView(self.p_str_ + start, stop - start);
    }

    [[nodiscard]] constexpr StringView trim() const noexcept{
        return trim_withfn(is_ascii_whitespace);
    }

    [[nodiscard]] constexpr explicit operator std::string_view() const noexcept{
        return std::string_view(p_str_, length_);
    }

    [[nodiscard]] std::span<const uint8_t> as_bytes() const noexcept{
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(p_str_), length_);
    }

    [[nodiscard]] std::span<const unsigned char> as_uchars() const noexcept{
        return std::span<const unsigned char>(reinterpret_cast<const unsigned char *>(p_str_), length_);
    }

    [[nodiscard]] constexpr std::span<const char> chars() const noexcept{
        return std::span<const char>(p_str_, length_);
    }
private:
    const char * p_str_;
    size_t length_;

    static constexpr bool is_ascii_whitespace(char ch) noexcept{ 
        // https://doc.rust-lang.org/std/primitive.u8.html#method.is_ascii_whitespace

        // Checks if the value is an ASCII whitespace character: U+0020 SPACE, 
        // U+0009 HORIZONTAL TAB, U+000A LINE FEED, U+000C FORM FEED, or U+000D CARRIAGE RETURN.
        
        switch(ch){
            case ' ':
            case '\t':
            case '\n':
            case '\f':
            case '\r':
                return true;
            default:
                return false;
        }
    }

};


}