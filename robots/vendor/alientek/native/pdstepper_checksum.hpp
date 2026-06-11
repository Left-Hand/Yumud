#pragma once

#include <cstdint>
#include <span>


namespace ymd::robots::pdstepper{


struct [[nodiscard]] ChecksumBuilder final{
    uint8_t checksum;

    static constexpr ChecksumBuilder from_default(){
        return ChecksumBuilder{.checksum = 0};
    }


    constexpr ChecksumBuilder push_byte(const uint8_t byte) const noexcept {
        ChecksumBuilder self = *this;
        self.checksum = static_cast<uint8_t>(self.checksum + byte);
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