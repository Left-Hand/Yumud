#pragma once

#include <span>
#include <cstdint>

namespace ymd::robots::waveshare::st3215{

struct [[nodiscard]] ChecksumBuilder final{
    using Self = ChecksumBuilder;

    static constexpr Self from_default(){
        Self self;
        self.checksum = 0;
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
        self.checksum = static_cast<uint8_t>(self.checksum + byte);

        return self;
    }

    [[nodiscard]] constexpr uint16_t finalize() const noexcept {
        return ~checksum;
    }

private:
    uint8_t checksum;
};


}