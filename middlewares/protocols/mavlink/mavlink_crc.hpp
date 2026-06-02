#pragma once

#include <cstdint>
#include <span>


namespace ymd::mavlink{



struct [[nodiscard]] Mcrf4xxChecksumBuilder final{
    using Self = Mcrf4xxChecksumBuilder;

    static constexpr Self from_default(){
        Self self;
        self.checksum = 0xffff;
        return self;
    }
    
    constexpr Self push_bytes(std::span<const uint8_t> bytes) const noexcept {
        Self self = *this;

        #pragma GCC unroll 4
        for(size_t i = 0; i < bytes.size(); i++) {
            self = self.push_byte(bytes[i]);
        }

        return self;
    }


    __attribute__((always_inline))
    constexpr Self push_byte(const uint8_t byte) const noexcept {
        Self self = *this;

        uint8_t tmp = byte ^ (self.checksum & 0xFF);
        tmp ^= tmp << 4;
        self.checksum = (self.checksum >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4);

        return self;
    }

    [[nodiscard]] constexpr uint16_t finalize() const noexcept {
        return checksum;
    }

    [[nodiscard]] constexpr std::array<uint8_t, 2> finalize_to_u8x2() const noexcept {
        return std::bit_cast<std::array<uint8_t, 2>>(checksum);
    }

private:
    uint16_t checksum;
};

}