#pragma once

#include <cstdint>
#include <span>

namespace ymd::drivers::tamagawa{

// 多项式：G(X)=X^8+1 LSB first  Poly: 0000 0001
// LSB first  : 1000 0000 =0X80
struct [[nodiscard]] ChecksumBuilder final {
    using Self = ChecksumBuilder;
    static constexpr uint8_t POLY = 0x01;

    static constexpr Self from_default(){
        Self self;
        self.checksum = 0;
        return self;
    }

    constexpr Self push_bytes(std::span<const uint8_t> bytes) const noexcept {
        auto self = *this;


        for (size_t i = 0; i < bytes.size(); ++i) {
            self = self.push_byte(bytes[i]);
        }
        return self;
    }

    constexpr Self push_byte(uint8_t byte) const noexcept {
        auto self = *this;
        self.checksum ^= byte;
        for (size_t i = 0; i < 8; ++i) {
            if (self.checksum & 0x80) {
                self.checksum = (self.checksum << 1) ^ POLY;
            } else {
                self.checksum <<= 1;
            }
        }
        return self;
    }

    [[nodiscard]] constexpr uint8_t finalize() const noexcept {
        return checksum;
    }
private:
    uint8_t checksum;
};



}