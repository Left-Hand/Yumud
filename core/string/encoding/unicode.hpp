#pragma once

#include <cstdint>
#include "core/string/view/string_view.hpp"

namespace ymd {

struct [[nodiscard]] UnicodeChar final{
    using Self = UnicodeChar;

    uint32_t bits;

    constexpr explicit UnicodeChar(uint32_t _bits):
        bits(_bits){;}

    static constexpr Self zero() {
        return Self{0};
    }
    constexpr bool operator==(const UnicodeChar & other) const {
        return bits == other.bits;
    }

    constexpr uint32_t count() const {
        return bits;
    }
};

struct [[nodiscard]] UnicodeIterator final{

public:
    constexpr UnicodeIterator(const StringView str):
        str_(str){;}
    [[nodiscard]] constexpr bool has_next() const {
        return offset_ < str_.length();
    }

    [[nodiscard]] constexpr UnicodeChar next() {
        if (this->has_next() == false) {
            return UnicodeChar::zero();
        }

        uint32_t unicodeValue = 0;
        uint8_t ch = static_cast<uint8_t>(str_[offset_]);

        if ((ch & 0x80) == 0) {
            // 1-byte UTF-8 character
            unicodeValue = ch;
            ++offset_;
        } else if ((ch & 0xE0) == 0xC0) {
            // 2-byte UTF-8 character
            unicodeValue = (ch & 0x1F) << 6;
            unicodeValue |= (static_cast<uint8_t>(str_[offset_ + 1]) & 0x3F);
            offset_ += 2;
        } else if ((ch & 0xF0) == 0xE0) {
            // 3-byte UTF-8 character
            unicodeValue = (ch & 0x0F) << 12;
            unicodeValue |= (static_cast<uint8_t>(str_[offset_ + 1]) & 0x3F) << 6;
            unicodeValue |= (static_cast<uint8_t>(str_[offset_ + 2]) & 0x3F);
            offset_ += 3;
        } else if ((ch & 0xF8) == 0xF0) {
            // 4-byte UTF-8 character
            unicodeValue = (ch & 0x07) << 18;
            unicodeValue |= (static_cast<uint8_t>(str_[offset_ + 1]) & 0x3F) << 12;
            unicodeValue |= (static_cast<uint8_t>(str_[offset_ + 2]) & 0x3F) << 6;
            unicodeValue |= (static_cast<uint8_t>(str_[offset_ + 3]) & 0x3F);
            offset_ += 4;
        } else {
            // Invalid UTF-8 sequence
            ++offset_;
        }

        return UnicodeChar(unicodeValue);
    }
private:
    StringView str_;
    size_t offset_;

};

}