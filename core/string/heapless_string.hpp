#pragma once

#include <cstdint>
#include "core/string/string_view.hpp"
#include "core/stream/ostream.hpp"

#include "core/utils/Result.hpp"

namespace ymd{

template<size_t N>
struct [[nodiscard]] HeaplessString{
    constexpr HeaplessString():
        len_(0){;}

    constexpr HeaplessString(const StringView str):
        len_(MIN(str.length(), N))
    {
        // memcpy(buf_, str.data(), len_);
        for(size_t i = 0; i < len_; i++){
            buf_[i] = str.data()[i];
        }
    }

    [[nodiscard]] static constexpr HeaplessString<N> from_str(const StringView str){
        return HeaplessString<N>(str);
    } 

    [[nodiscard]] static constexpr HeaplessString<N> from_empty(){
        return HeaplessString<N>();
    } 

    [[nodiscard]] constexpr char operator [](const size_t idx) const {return buf_[idx];}
    [[nodiscard]] constexpr char & operator [](const size_t idx){return buf_[idx];}


    [[nodiscard]] constexpr StringView view() const {
        return StringView(buf_, len_);
    }

    [[nodiscard]] constexpr const char * data() const {
        return buf_;
    }

    [[nodiscard]] constexpr Result<void, void> push_back(const char chr){
        if(len_ >= N) return Err();
        buf_[len_++] = chr;
        return Ok();
    }

    [[nodiscard]] constexpr Result<char, void> pop_back(){
        if(len_ == 0) return Err();
        return Ok(buf_[--len_]);
    }

    constexpr void push_back_unchecked(const char chr){
        if(len_ >= N) return;
        buf_[len_++] = chr;
    }

    [[nodiscard]] constexpr size_t length() const{
        return len_;
    }

    [[nodiscard]] constexpr size_t size() const{
        return len_;
    }

    constexpr void insert_unchecked(const size_t idx, const char chr){
        if (len_ >= N or idx > len_) return;
        // Move characters from the end to make space for the new character
        for (size_t i = len_; i > idx; i--) {
            buf_[i] = buf_[i - 1];
        }
        // Insert the new character at the specified index
        buf_[idx] = chr;
        // Increase the length of the string
        len_++;
    }

    [[nodiscard]] constexpr Result<void, void> try_insert(const size_t idx, const char chr){
        if (len_ >= N || idx > len_) return Err();
        // Move characters from the end to make space for the new character
        for (size_t i = len_; i > idx; i--) {
            buf_[i] = buf_[i - 1];
        }
        // Insert the new character at the specified index
        buf_[idx] = chr;
        // Increase the length of the string
        len_++;

        return Ok();
    }

    [[nodiscard]] constexpr Result<void, void> try_erase(const size_t idx){
        if (idx > len_) return Err();  // 正确索引检查
        if (len_ == 0) return Err();    // 防止空字符串操作
        if (idx == 0) return Err();

        // 前移字符（注意循环终止条件）
        for (size_t i = idx-1; i < len_ - 1; i++) {
            buf_[i] = buf_[i + 1];
        }

        len_--;  // 必须更新长度
        return Ok();
    }

    constexpr void clear(){
        for(size_t i = 0; i < len_; i++)
            buf_[i] = 0;
        len_ = 0;
    }

    [[nodiscard]] constexpr std::span<char> mut_chars() noexcept{
        return std::span<char>(buf_, len_);
    }

    [[nodiscard]] constexpr std::span<const char> chars() const noexcept{
        return std::span<char>(buf_, len_);
    }

    [[nodiscard]] constexpr bool operator==(const StringView & other) const {
        return this->str() == other;
    } 

    friend OutputStream & operator<<(OutputStream & os, const HeaplessString<N> self) {
        return os << self.view();
    }

private:

    char buf_[N] = {};
    size_t len_;

    #if 0
    static consteval void static_test(){
        constexpr auto str = HeaplessString<10>("Hello");
        constexpr auto str2 = []{
            auto _str = HeaplessString<10>("Hello");
            _str.push_back('!');
            return _str;
        }();

        constexpr auto str3 = []{
            auto _str = HeaplessString<10>("Hello");
            _str.insert(0, '!');
            return _str;
        }();

        constexpr auto str4 = []{
            auto _str = HeaplessString<10>("Hello");
            _str.erase(4);
            return _str;
        }();

        static_assert(str.length() == 5);
        static_assert(str2.length() == 6);
        static_assert(str3.length() == 6);
        static_assert(str2 == StringView("Hello!"));
        static_assert(str3 == StringView("!Hello"));
        static_assert(str4 == StringView("Hell"));

        // 测试越界插入
        static_assert([]{
            auto str = HeaplessString<5>("Hi");
            str.insert(3, '!');  // 允许在 len_=2 的索引3插入
            return str == StringView("Hi!");  // ✅ 应成功
        }());

        // 测试无效插入返回错误
        static_assert(
            HeaplessString<3>("A").insert(5, 'X').is_err()  // ✅ 应返回错误
        );
    }
    #endif
};
}