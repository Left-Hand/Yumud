#pragma once

#include <cstdint>
#include <span>

namespace ymd::volz{

struct [[nodiscard]] ChecksumBuilder final{
    uint16_t checksum;

    static constexpr ChecksumBuilder from_default(){
        return ChecksumBuilder{.checksum = 0xffff};
    }


    constexpr ChecksumBuilder push_byte(const uint8_t byte) const noexcept {
        ChecksumBuilder self = *this;

        self.checksum = (byte << 8) ^ self.checksum;

        for(size_t j = 0; j < 8; j++) {
            if (self.checksum & 0x8000) {
                self.checksum = (self.checksum << 1) ^ 0x8005;
            } else {
                self.checksum = self.checksum << 1;
            }
        }
        return self;
    }

    constexpr ChecksumBuilder push_bytes(std::span<const uint8_t> bytes) const noexcept {
        ChecksumBuilder self = *this;
        for(size_t i = 0; i < bytes.size(); i++){
            self = self.push_byte(bytes[i]);
        }
        return self;
    }

    [[nodiscard]] uint8_t finalize() const noexcept {
        return checksum;
    }
};

}

