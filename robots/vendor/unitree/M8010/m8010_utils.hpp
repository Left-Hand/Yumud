#pragma once


#include "core/utils/bytes/buffer_cursor.hpp"

namespace ymd::robots::unitree::m8010{


struct [[nodiscard]] Crc16Builder final{
    using Self = Crc16Builder;


    static constexpr Crc16Builder from_default(){
        Self self;
        self.checksum = 0;
        return self;
    }

    constexpr Crc16Builder push_bytes(std::span<const uint8_t> bytes) const noexcept {
        auto self = *this;

        const uint8_t *data = bytes.data();
        size_t length = bytes.size();
        uint8_t i;
        while(length--)
        {
            self.checksum ^= *data++;        // self.checksum ^= *data; data++;
            for (i = 0; i < 8; ++i)
            {
                if (self.checksum & 1)
                    self.checksum = (self.checksum >> 1) ^ 0x8408;        // 0x8408 = reverse 0x1021
                else
                    self.checksum = (self.checksum >> 1);
            }
        }

        return self;
    }

    [[nodiscard]] constexpr uint32_t finalize() const noexcept {
        return checksum;
    }

private:
    uint16_t checksum;

};

}