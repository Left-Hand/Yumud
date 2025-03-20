#pragma once

#include <cstdint>

namespace ymd {

class UnicodeIterator {
private:
    const char* utf8String;
    size_t currentIndex;
    size_t length;

public:
    UnicodeIterator(const char* _utf8String) : utf8String(_utf8String), currentIndex(0) {
        length = 0;
        while (utf8String[length] != '\0') {
            ++length;
        }
    }

    operator bool() const {
        return currentIndex < length;
    }

    uint32_t next() {
        if (this->operator bool() == false) {
            return -1;
        }

        uint32_t unicodeValue = 0;
        uint8_t ch = static_cast<uint8_t>(utf8String[currentIndex]);

        if ((ch & 0x80) == 0) {
            // 1-byte UTF-8 character
            unicodeValue = ch;
            ++currentIndex;
        } else if ((ch & 0xE0) == 0xC0) {
            // 2-byte UTF-8 character
            unicodeValue = (ch & 0x1F) << 6;
            unicodeValue |= (static_cast<uint8_t>(utf8String[currentIndex + 1]) & 0x3F);
            currentIndex += 2;
        } else if ((ch & 0xF0) == 0xE0) {
            // 3-byte UTF-8 character
            unicodeValue = (ch & 0x0F) << 12;
            unicodeValue |= (static_cast<uint8_t>(utf8String[currentIndex + 1]) & 0x3F) << 6;
            unicodeValue |= (static_cast<uint8_t>(utf8String[currentIndex + 2]) & 0x3F);
            currentIndex += 3;
        } else if ((ch & 0xF8) == 0xF0) {
            // 4-byte UTF-8 character
            unicodeValue = (ch & 0x07) << 18;
            unicodeValue |= (static_cast<uint8_t>(utf8String[currentIndex + 1]) & 0x3F) << 12;
            unicodeValue |= (static_cast<uint8_t>(utf8String[currentIndex + 2]) & 0x3F) << 6;
            unicodeValue |= (static_cast<uint8_t>(utf8String[currentIndex + 3]) & 0x3F);
            currentIndex += 4;
        } else {
            // Invalid UTF-8 sequence
            ++currentIndex;
        }

        return unicodeValue;
    }
};

}