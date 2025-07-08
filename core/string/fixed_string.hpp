#pragma once

#include <cstdint>
#include "core/string/StringView.hpp"
#include "core/stream/ostream.hpp"

#include "core/utils/Result.hpp"

namespace ymd{

template<size_t N>
struct FixedString{
    constexpr FixedString():
        len_(0){;}

    constexpr FixedString(const StringView str):
        len_(MIN(str.length(), N))
    {
        // memcpy(buf_, str.data(), len_);
        for(size_t i = 0; i < len_; i++){
            buf_[i] = str.data()[i];
        }
    }


    [[nodiscard]] constexpr StringView as_view() const {
        return StringView(buf_, len_);
    }

    [[nodiscard]] constexpr Result<void, void> try_push_back(const char chr){
        if(len_ >= N) return Err();
        buf_[len_++] = chr;
        return Ok();
    }

    [[nodiscard]] constexpr Result<char, void> try_pop_back(){
        if(len_ == 0) return Err();
        return Ok(buf_[--len_]);
    }

    [[nodiscard]] constexpr size_t length() const{
        return len_;
    }

    [[nodiscard]] constexpr size_t size() const{
        return len_;
    }

    constexpr void insert(const size_t idx, const char chr){
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

    [[nodiscard]]constexpr bool operator==(const StringView & other) const {
        return len_ == other.length() && std::memcmp(buf_, other.data(), len_) == 0;
    } 

    friend OutputStream & operator<<(OutputStream & os, const FixedString<N> self) {
        return os << self.as_view();
    }

private:

    char buf_[N] = {};
    size_t len_;

    #if 0
    static consteval void static_test(){
        constexpr auto str = FixedString<10>("Hello");
        constexpr auto str2 = []{
            auto _str = FixedString<10>("Hello");
            _str.try_push_back('!');
            return _str;
        }();

        constexpr auto str3 = []{
            auto _str = FixedString<10>("Hello");
            _str.insert(0, '!');
            return _str;
        }();

        constexpr auto str4 = []{
            auto _str = FixedString<10>("Hello");
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
            auto str = FixedString<5>("Hi");
            str.try_insert(3, '!');  // 允许在 len_=2 的索引3插入
            return str == StringView("Hi!");  // ✅ 应成功
        }());

        // 测试无效插入返回错误
        static_assert(
            FixedString<3>("A").insert(5, 'X').is_err()  // ✅ 应返回错误
        );
    }
    #endif
};
}