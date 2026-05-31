#pragma once

#include <cstdint>
#include <span>


namespace ymd::mavlink{

struct [[nodiscard]] ChecksumBuilder final{
    using Self = ChecksumBuilder;

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

        self.checksum ^= static_cast<uint16_t>(byte) << 8;
        for (int i = 0; i < 8; ++i) {
            if (self.checksum & 0x8000) {
                self.checksum = (self.checksum << 1) ^ 0x1021; // 多项式 0x1021
            } else {
                self.checksum <<= 1;
            }
        }

        return self;
    }

    [[nodiscard]] constexpr uint16_t finalize() const noexcept {
        return checksum;
    }

private:
    uint16_t checksum;
};

}