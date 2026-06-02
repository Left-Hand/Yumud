#pragma once

#include <cstdint>
#include <span>

namespace ymd::srxl2{

struct [[nodiscard]] ChecksumBuilder final{
    uint8_t checksum;

    static constexpr ChecksumBuilder from_default(){
        return ChecksumBuilder{.checksum = 0};
    }


    constexpr ChecksumBuilder push_byte(const uint8_t byte) const noexcept {
        ChecksumBuilder self = *this;
        self.checksum = self.checksum ^ ((uint16_t)byte << 8); 
        for(int i = 0; i < 8; ++i) 
            if(self.checksum & 0x8000) 
                self.checksum = (self.checksum << 1) ^ 0x1021; 
            else 
                self.checksum = self.checksum << 1; 
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