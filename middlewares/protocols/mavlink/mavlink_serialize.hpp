#pragma once


#include "mavlink_crc.hpp"

// https://mavlink.io/en/guide/serialization.html


namespace ymd::mavlink{

static constexpr MAVLINK_V1_HEADER = 0xfe;
static constexpr MAVLINK_V2_HEADER = 0xfd;

struct [[nodiscard]] MavlinkHeaderV1{
    const uint8_t header = MAVLINK_V1_HEADER;
    uint8_t len;
    uint8_t seq;
    uint8_t sysid;
    uint8_t compid;
    uint8_t msgid;

    template<typename Serializer>
    Result<void, typename Serializer::Error> 
    serialize(Serializer & serializer) const noexcept{
        auto & self = *this;

        const uint8_t buf[] = {
            MAVLINK_V1_HEADER,
            self.len,
            self.seq,
            self.sysid,
            self.compid,
            self.msgid
        };

        return serializer.push_bytes(std::span(buf));
    }
};

static_assert(sizeof(MavlinkHeaderV1) == 6);


struct [[nodiscard]] MavlinkHeaderV2{
    const uint8_t header = MAVLINK_V2_HEADER;
    uint8_t len;
    uint8_t incompat_flags;
    uint8_t compat_flags;
    uint8_t seq;
    uint8_t sysid;
    uint8_t compid;
    uint32_t msgid;

    template<typename Serializer>
    Result<void, typename Serializer::Error> 
    serialize(Serializer & serializer) const noexcept{
        auto & self = *this;

        const uint8_t buf[] = {
            MAVLINK_V2_HEADER,
            self.len,
            self.incompat_flags,
            self.compat_flags,
            self.seq,
            self.sysid,
            self.compid,
            static_cast<uint8_t>(self.msgid),
            static_cast<uint8_t>(self.msgid >> 8),
            static_cast<uint8_t>(self.msgid >> 16),
        };

        return serializer.push_bytes(std::span(buf));
    }
};

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