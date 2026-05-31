#pragma once


#include "mavlink_crc.hpp"

namespace ymd::mavlink{

struct [[nodiscard]] MavlinkHeaderV1{
    const uint8_t header = 0xFE;
    uint8_t len;
    uint8_t seq;
    uint8_t sysid;
    uint8_t compid;
    uint8_t msgid;

    [[nodiscard]] constexpr std::span<const uint8_t> as_bytes() const noexcept {
        return std::span<const uint8_t>(&header, 6);
    }
};

static_assert(sizeof(MavlinkHeaderV1) == 6);

struct [[nodiscard]] MavlinkTrailerV1{
    uint8_t crc_low;
    uint8_t crc_high;

    static constexpr MavlinkTrailerV1 from_bytes(std::span<const uint8_t> buf){
        const auto crc = ChecksumBuilder::from_default()
            .push_bytes(buf)
            .finalize();

        const uint8_t low = crc & 0xFF;
        const uint8_t high = (crc >> 8) & 0xFF;
        return MavlinkTrailerV1{low, high};
    }



    [[nodiscard]] constexpr std::span<const uint8_t> as_bytes() const noexcept {
        return std::span<const uint8_t>(&crc_low, 2);
    }
};

struct [[nodiscrad]] MavlinkFrame{
    MavlinkHeaderV1 header;
    std::span<const uint8_t> payload;
    MavlinkTrailerV1 trailer;
};

}