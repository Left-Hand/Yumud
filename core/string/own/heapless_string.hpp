#pragma once

#include <cstdint>
#include "core/string/view/string_view.hpp"
#include "core/stream/ostream.hpp"

#include "core/utils/Result.hpp"

namespace ymd{

template<size_t N>
struct [[nodiscard]] HeaplessString final{
    constexpr HeaplessString():
        length_(0) {;}

    constexpr explicit HeaplessString(const StringView str):
        length_(MIN(str.length(), N)){
        #pragma GCC unroll 4
        for(size_t i = 0; i < length_; i++){
            buf_[i] = str.data()[i];
        }
    }

    constexpr ~HeaplessString() = default;

    [[nodiscard]] static constexpr HeaplessString<N> from_str(const StringView str) noexcept{
        return HeaplessString<N>(str);
    } 

    [[nodiscard]] static constexpr HeaplessString<N> from_empty() noexcept {
        return HeaplessString<N>();
    } 

    [[nodiscard]] constexpr char operator [](const size_t idx) const noexcept{
        return buf_[idx];
    }

    [[nodiscard]] constexpr char & operator [](const size_t idx) noexcept{
        return buf_[idx];
    }

    [[nodiscard]] constexpr char at(const size_t idx) const noexcept{
        if(idx >= length_) __builtin_trap();
        return buf_[idx];
    }
    [[nodiscard]] constexpr char & at(const size_t idx) noexcept{
        if(idx >= length_) __builtin_trap();
        return buf_[idx];
    }

    [[nodiscard]] constexpr const char * data() const noexcept{
        return buf_.data();
    }

    [[nodiscard]] constexpr Result<void, void> push_back(const char chr) noexcept{
        if(length_ >= N) return Err();
        buf_[length_++] = chr;
        return Ok();
    }

    [[nodiscard]] constexpr Result<char, void> pop_back() noexcept{
        if(length_ == 0) return Err();
        return Ok(buf_[--length_]);
    }

    constexpr void push_back_unchecked(const char chr) noexcept{
        if(length_ >= N) return;
        buf_[length_++] = chr;
    }

    [[nodiscard]] constexpr size_t length() const noexcept{
        return length_;
    }

    [[nodiscard]] constexpr size_t size() const noexcept{
        return length_;
    }

    constexpr void insert_unchecked(const size_t idx, const char chr) noexcept{
        if (length_ >= N or idx > length_) return;
        // Move characters from the end to make space for the new character
        for (size_t i = length_; i > idx; i--) {
            buf_[i] = buf_[i - 1];
        }
        // Insert the new character at the specified index
        buf_[idx] = chr;
        // Increase the length of the string
        length_++;
    }

    [[nodiscard]] constexpr Result<void, void> try_insert(const size_t idx, const char chr) noexcept{
        if (length_ >= N || idx > length_) return Err();
        // Move characters from the end to make space for the new character
        for (size_t i = length_; i > idx; i--) {
            buf_[i] = buf_[i - 1];
        }
        // Insert the new character at the specified index
        buf_[idx] = chr;
        // Increase the length of the string
        length_++;

        return Ok();
    }

    [[nodiscard]] constexpr Result<void, void> try_erase(const size_t idx) noexcept{
        if (idx > length_) return Err();  // 正确索引检查
        if (length_ == 0) return Err();    // 防止空字符串操作
        if (idx == 0) return Err();

        // 前移字符（注意循环终止条件）
        for (size_t i = idx-1; i < length_ - 1; i++) {
            buf_[i] = buf_[i + 1];
        }

        length_--;  // 必须更新长度
        return Ok();
    }

    constexpr void clear() noexcept {
        length_ = 0;
    }

    [[nodiscard]] constexpr std::span<char> mut_chars() noexcept{
        return std::span<char>(buf_.data(), length_);
    }

    [[nodiscard]] constexpr std::span<const char> chars() const noexcept{
        return std::span<const char>(buf_.data(), length_);
    }

    [[nodiscard]] constexpr std::span<uint8_t> mut_uchars() noexcept{
        return std::span<uint8_t>(
            reinterpret_cast<uint8_t*>(buf_.data()), 
            length_
        );
    }

    [[nodiscard]] constexpr std::span<const uint8_t> uchars() const noexcept{
        return std::span<const uint8_t>(
            reinterpret_cast<const uint8_t*>(buf_.data()), 
            length_
        );
    }

    [[nodiscard]] constexpr StringView view() const noexcept{
        return StringView(buf_.data(), length_);
    }

    friend OutputStream & operator<<(OutputStream & os, const HeaplessString<N> self) noexcept{
        return os << self.view();
    }

private:
    std::array<char, N> buf_ = {};
    size_t length_;
};
}