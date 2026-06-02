#pragma once


#include "mavlink_crc.hpp"
#include "mavlink_port.hpp"
#include "core/utils/Result.hpp"

// https://mavlink.io/en/guide/serialization.html

// 注意，多字节字段以小端格式序列化，MAVLink库默认配置为在小端硬件上运行



namespace ymd::mavlink{

static constexpr uint8_t MAVLINK_V1_HEADER = 0xfe;
static constexpr uint8_t MAVLINK_V2_HEADER = 0xfd;

static constexpr size_t MIN_V1_FRAME_LENGTH = 8;
static constexpr size_t MAX_V1_FRAME_LENGTH = 263;

static constexpr size_t MIN_V2_FRAME_LENGTH = 12;
static constexpr size_t MAX_V2_FRAME_LENGTH = 280;

static constexpr size_t SIGNATURE_LENGTH = 13;

struct [[nodiscard]] MavlinkHeaderV1 final{
    // 表示下一节的长度（针对特定消息固定）。payload
    uint8_t len;

    // 用于检测丢包。组件对每条发送的消息进行增值。
    uint8_t seq;

    // (0, 255]发送消息的系统（载具）识别码。用于区分网络上的系统。
    // 注意，广播地址0不能在此字段中使用，因为它是一个无效的源地址。
    uint8_t sysid;

    // (0, 255] 发送消息的组件编号。用于区分系统中的各个组件（例如自动驾驶仪和摄像头）。
    // MAV_COMPONENT中使用合适的数值。请注意，广播地址不能用于该字段，因为它是一个无效的源地址。
    uint8_t compid;

    // [0, 255] payload中的消息类型ID。用于将数据解码回消息对象
    uint8_t msgid;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & serializer) const noexcept{
        auto & self = *this;

        #ifndef _NDEBUG
        MAVLINK_DEBUG_ASSERT(sysid != 0, "sysid can't be 0")
        MAVLINK_DEBUG_ASSERT(compid != 0, "compid can't be 0")
        #endif


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



struct [[nodiscard]] MavlinkHeaderV2 final{
    uint8_t len;
    uint8_t incompat_flags;
    uint8_t compat_flags;
    uint8_t seq;
    uint8_t sysid;
    uint8_t compid;
    uint32_t msgid;

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & serializer) const noexcept{
        auto & self = *this;

        #ifndef _NDEBUG
        MAVLINK_DEBUG_ASSERT((msgid >> 24) == 0, "sysid can't be 0")
        #endif

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



template<typename Serializer>
static constexpr Result<void, typename Serializer::Error> 
serialize_header_v1(
    Serializer & serializer, 
    const MavlinkHeaderV1 & header
) noexcept{
    if(const auto res = header.serialize(serializer);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

template<typename Serializer>
static constexpr Result<void, typename Serializer::Error> 
serialize_header_v2(
    Serializer & serializer, 
    const MavlinkHeaderV2 & header
) noexcept{
    if(const auto res = header.serialize(serializer);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

template<typename Serializer>
static constexpr Result<void, typename Serializer::Error> 
serialize_tailer(
    Serializer & serializer,

    //需要参与crc计算的字节
    std::span<const uint8_t> bytes,
    uint8_t crc_extra
) noexcept{
    const auto buf = Mcrf4xxChecksumBuilder::from_default()
        .push_bytes(bytes)
        .push_byte(crc_extra)
        .finalize_to_u8x2();

    if(const auto res = serializer.push_bytes(std::span(buf));
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


template<typename Serializer>
static constexpr Result<void, typename Serializer::Error> 
serialize_signature(
    Serializer & serializer, 
    std::span<const uint8_t, SIGNATURE_LENGTH> signature
) noexcept{
    if(const auto res = (serializer.push_bytes(signature));
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}



}