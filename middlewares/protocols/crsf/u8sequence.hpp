#pragma once

#include <span>
#include <cstdint>

struct [[nodiscard]] U8Sequence final{
    using Self = U8Sequence;
    static constexpr Self from_chars(std::span<const char> chars){
        Self ret;
        ret.p_chars_ = chars.data();
        ret.length_ = chars.size();
        return ret;
    }

    static constexpr Self from_bytes(std::span<const uint8_t> bytes){
        Self ret;
        ret.p_bytes_ = bytes.data();
        ret.length_ = bytes.size();
        return ret;
    }

    [[nodiscard]] constexpr uint8_t operator[](size_t idx) const noexcept {return p_bytes_[idx];}
    [[nodiscard]] constexpr const uint8_t char_at(size_t idx) const noexcept {
        if(idx >= length_) __builtin_trap();
        return p_chars_[idx];
    }

    [[nodiscard]] constexpr const uint8_t byte_at(size_t idx) const noexcept {
        if(idx >= length_) __builtin_trap();
        return p_bytes_[idx];
    }
    [[nodiscard]] constexpr size_t length() const noexcept {return length_;}
    [[nodiscard]] constexpr size_t size() const noexcept {return length_;}
    [[nodiscard]] constexpr std::span<const char> as_chars() const noexcept {return {p_chars_, length_};}
    [[nodiscard]] constexpr std::span<const uint8_t> as_bytes() const noexcept {return {p_bytes_, length_};}

private:
    union{
        const char * p_chars_;
        const uint8_t * p_bytes_;
    };
    size_t length_;
};
