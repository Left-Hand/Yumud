#pragma once

#include <cstdint>
#include <span>

namespace ymd::robots::mwd{


struct [[nodiscard]] WrapAddAccumulator final{
    using Self = WrapAddAccumulator;
    uint8_t val;

    static constexpr WrapAddAccumulator from_default(){
        return WrapAddAccumulator{.val = 0};
    }


    constexpr Self push_byte(const uint8_t byte) const {
        Self self = *this;
        self.val = static_cast<uint8_t>(self.val + byte);
        return self;
    }

    constexpr Self push_bytes(const std::span<const uint8_t> bytes) const {
        Self self = *this;
        for(const auto byte : bytes){
            self.val = static_cast<uint8_t>(self.val + byte);
        }
        return self;
    }

    [[nodiscard]] constexpr uint8_t finalize() const {
        return val;
    }
};
}