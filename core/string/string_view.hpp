#pragma once

#include "core/utils/hash_func.hpp"
#include "core/utils/option.hpp"

namespace ymd{

class String;
class StringView;

#if 0
struct NullableStringView{
    static constexpr NullableStringView from_null(){
        return NullableStringView(nullptr);
    }

    constexpr NullableStringView() = delete;
    constexpr NullableStringView(const char * str) = delete;

    constexpr explicit NullableStringView(std::nullptr_t):
        data_(nullptr),
        size_(0){}

    constexpr NullableStringView(_None_t):
        data_(nullptr),
        size_(0){}

    constexpr NullableStringView(const StringView & str):
        data_(str.data()),
        size_(str.length()){}

    constexpr NullableStringView(Some<StringView> str):
        NullableStringView(str.get()){;}

    constexpr NullableStringView(Option<StringView> may_str){
        if(may_str.is_some()){
            const auto str = may_str.unwrap();
            data_ = str.data();
            size_ = str.size();
        }else{
            data_ = nullptr;
            size_ = 0;
        }
    }


    constexpr explicit NullableStringView(const NullableStringView & other):
        data_(other.data_),
        size_(other.size_){}
    constexpr explicit NullableStringView(NullableStringView && other):
        data_(other.data_),
        size_(other.size_){}

    constexpr bool is_null() const{return size_==0;}
    constexpr bool is_none() const{return size_==0;}
    constexpr bool is_some() const{return size_!=0;}

    constexpr StringView unwrap_or(const StringView default_str) const{
        if(unlikely(size_ == 0)) return default_str;
        return unwrap();
    }

    constexpr StringView unwrap() const{
        if(unlikely(size_ == 0)) __builtin_trap();
        return StringView(data_, size_);
    }

    constexpr StringView unwrap_unchecked() const{
        return StringView(data_, size_);
    }

    constexpr size_t length() const{
        return size_;
    }

private:
    const char * data_;
    size_t size_;
};
#endif

class [[nodiscard]] StringView {
public:
    // 构造函数 从容器构造必须为显式 避免调用者没注意到生命周期
    constexpr explicit StringView(const std::string & str) noexcept: 
        data_(str.data()), size_(str.length()) {}
    constexpr StringView(const std::string_view str) noexcept: 
        data_(str.data()), size_(str.length()) {}


    //从不原始c字符串构造考虑到不带尾0的攻击 只能在编译期使用 运行期使用from_cstr
    consteval StringView(const char * p_chars) noexcept: 
        data_(p_chars), size_((p_chars != nullptr) ? strlen(p_chars) : 0) {}

    constexpr StringView(const char * p_chars, size_t size) noexcept: 
        data_(p_chars), size_(size) {}

    static constexpr StringView from_cstr(const char * p_chars, size_t max_size = std::dynamic_extent) noexcept{
        return from_zero_terminated(p_chars, max_size);
    }
    
    static constexpr StringView from_zero_terminated(const char * p_chars, size_t max_size = std::dynamic_extent) noexcept{
        const size_t size = (p_chars != nullptr) ? strnlen(p_chars, max_size) : 0;
        return StringView(p_chars, size);
    }

    template<size_t N>
    constexpr StringView(const char (&str)[N]) noexcept:
        data_(str), size_(strnlen(str, N)){}
    constexpr StringView(const StringView & other) noexcept: 
        data_(other.data_), size_(other.size_){;}
    constexpr StringView(StringView && other) noexcept: 
        data_(other.data_), size_(other.size_){;}

    constexpr StringView& operator=(const StringView & other) noexcept{
        data_ = other.data_;
        size_ = other.size_;
        return *this;
    }

    constexpr StringView& operator=(StringView && other) noexcept{
        data_ = std::move(other.data_);
        size_ = std::move(other.size_);
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const StringView & other) const noexcept{ 

        if(size_ != other.size_) return false;

        #pragma GCC unroll 4
        for(size_t i = 0; i < size_; i++){
            if(data_[i] != other.data_[i]) return false;
        }

        return true;
    }

    [[nodiscard]] constexpr const char * begin() const noexcept{return data_;}
    [[nodiscard]] constexpr const char * end() const noexcept{return data_ + size_;}

    [[nodiscard]] constexpr size_t size() const noexcept{ return size_; }
    [[nodiscard]] constexpr size_t length() const noexcept{return size_;}

    [[nodiscard]] constexpr const char* data() const noexcept{ return data_; }

    [[nodiscard]] constexpr char operator [](const size_t index) const noexcept{return data_[index];}
	__fast_inline constexpr Option<StringView> substr(size_t left) const noexcept{
        return substr_by_range(left, size_);};
	__fast_inline constexpr Option<StringView> substr_by_range(size_t left, size_t right) const noexcept{
        if (unlikely(left > right)) 
            return None;
        
        if (right > size_) 
            return None;

        return Some(StringView(this->data_ + left, right - left));
    }

	__fast_inline constexpr Option<StringView> substr_by_len(size_t left, size_t len) const noexcept{
        if (left + len > size_) 
            return None;

        return Some(StringView(this->data_ + left, len));
    }
    
    [[nodiscard]] constexpr Option<size_t> find(char c) const noexcept{
        return find_from(c, 0);
    }

    [[nodiscard]] constexpr Option<size_t> find_from(char ch, size_t from) const noexcept{
        if (from >= size_) return None;
        for(size_t i = from; i < size_; i++){
            if(data_[i] == ch) return Some(i);
        }
        return None;
    }


    [[nodiscard]] constexpr uint32_t hash() const noexcept{return ymd::hash(*this);}

    [[nodiscard]] constexpr StringView trim() const noexcept{
        auto & self = *this;
        auto is_whitespace = [](char c) {
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

        return StringView(
            self.data() + start, 
            static_cast<size_t>(MAX(static_cast<int>(end - start), 0))
        );
    }

    [[nodiscard]] constexpr operator std::string_view() const noexcept{
        return std::string_view(data_, size_);
    }

    [[nodiscard]] std::span<const uint8_t> as_bytes() const noexcept{
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(data_), size_);
    }

    [[nodiscard]] std::span<const unsigned char> as_uchars() const noexcept{
        return std::span<const unsigned char>(reinterpret_cast<const unsigned char *>(data_), size_);
    }

    [[nodiscard]] constexpr std::span<const char> chars() const noexcept{
        return std::span<const char>(data_, size_);
    }
private:
    const char * data_;
    size_t size_;
};


}