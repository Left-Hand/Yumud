
#pragma once

#include <cstdint>
#include <span>
#include "algo/encrypt/crc_common.hpp"

namespace ymd::ymodem{

struct [[nodiscard]] ChecksumBuilder final{
    static constexpr ChecksumBuilder from_default(){
        ChecksumBuilder self;
        self.checksum = 0;
        return self;
    }

    constexpr ChecksumBuilder push_byte(const uint8_t byte) const noexcept {
        ChecksumBuilder self = *this;
        self.checksum ^= byte;        // self.checksum ^= *data; data++;
        for (size_t i = 0; i < 8; ++i){
            if (self.checksum & 1)
                self.checksum = (self.checksum >> 1) ^ 0x8408;        // 0x8408 = reverse 0x1021
            else
                self.checksum = (self.checksum >> 1);
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
private:
    uint8_t checksum;
};

}