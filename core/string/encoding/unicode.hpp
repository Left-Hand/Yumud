#pragma once

#include <cstdint>
#include "core/string/view/string_view.hpp"

namespace ymd {

struct [[nodiscard]] UnicodeIterator final{

public:
    constexpr UnicodeIterator(const StringView str):
        str_(str){;}
    [[nodiscard]] constexpr bool has_next() const {
        return idx_ < length;
    }

    [[nodiscard]] constexpr uint32_t next() {
        if (this->has_next() == false) {
            return -1;
        }

        uint32_t unicodeValue = 0;
        uint8_t ch = static_cast<uint8_t>(str_[idx_]);

        if ((ch & 0x80) == 0) {
            // 1-byte UTF-8 character
            unicodeValue = ch;
            ++idx_;
        } else if ((ch & 0xE0) == 0xC0) {
            // 2-byte UTF-8 character
            unicodeValue = (ch & 0x1F) << 6;
            unicodeValue |= (static_cast<uint8_t>(str_[idx_ + 1]) & 0x3F);
            idx_ += 2;
        } else if ((ch & 0xF0) == 0xE0) {
            // 3-byte UTF-8 character
            unicodeValue = (ch & 0x0F) << 12;
            unicodeValue |= (static_cast<uint8_t>(str_[idx_ + 1]) & 0x3F) << 6;
            unicodeValue |= (static_cast<uint8_t>(str_[idx_ + 2]) & 0x3F);
            idx_ += 3;
        } else if ((ch & 0xF8) == 0xF0) {
            // 4-byte UTF-8 character
            unicodeValue = (ch & 0x07) << 18;
            unicodeValue |= (static_cast<uint8_t>(str_[idx_ + 1]) & 0x3F) << 12;
            unicodeValue |= (static_cast<uint8_t>(str_[idx_ + 2]) & 0x3F) << 6;
            unicodeValue |= (static_cast<uint8_t>(str_[idx_ + 3]) & 0x3F);
            idx_ += 4;
        } else {
            // Invalid UTF-8 sequence
            ++idx_;
        }

        return unicodeValue;
    }
private:
    const char* str_;
    size_t idx_;

};

}