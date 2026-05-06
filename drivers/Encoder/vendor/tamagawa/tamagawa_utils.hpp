#pragma once

#include <cstdint>
#include <span>

namespace ymd::drivers::tamagawa{

// 多项式：G(X)=X^8+1 LSB first  Poly: 0000 0001
// LSB first  : 1000 0000 =0X80
struct [[nodiscard]] Crc8XorAccumulator final {
    using Self = Crc8XorAccumulator;
    static constexpr uint8_t POLY = 0x01;

    uint8_t crc = 0;


    constexpr Self push_bytes(std::span<const uint8_t> bytes) const noexcept {
        auto self = *this;


        for (size_t i = 0; i < bytes.size(); ++i) {
            self = self.push_byte(bytes[i]);
        }
        return self;
    }

    constexpr Self push_byte(uint8_t byte) const noexcept {
        auto self = *this;
        self.crc ^= byte;
        for (size_t i = 0; i < 8; ++i) {
            if (self.crc & 0x80) {
                self.crc = (self.crc << 1) ^ POLY;
            } else {
                self.crc <<= 1;
            }
        }
        return self;
    }

    [[nodiscard]] constexpr uint8_t finalize() const noexcept {
        return crc;
    }
};



}