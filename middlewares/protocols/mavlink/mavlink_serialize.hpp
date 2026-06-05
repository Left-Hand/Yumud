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

static constexpr uint8_t MAVLINKV1_EXTRA_CRC_TABLE[256] = { 
    50, 124, 137, 0, 237, 217, 104, 119, 117, 0, 0,
	89, 0, 0, 0, 0, 0, 0, 0, 137, 214, 159, 220, 168, 24, 23, 170, 144, 67, 115, 39, 246, 185, 104, 
	237, 244, 222, 212, 9, 254, 230, 28, 28, 132, 221, 232, 11, 153, 41, 39, 78, 196, 132, 0, 15, 3,
	0, 0, 0, 0, 0, 167, 183, 119, 191, 118, 148, 21, 0, 243, 124, 0, 0, 38, 20, 158, 152, 143, 0, 0,
	14, 106, 49, 22, 143, 140, 5, 150, 0, 231, 183, 63, 54, 47, 0, 0, 0, 0, 0, 0, 175, 102, 158, 208,
	56, 93, 138, 108, 32, 185, 84, 34, 174, 124, 237, 4, 76, 128, 56, 116, 134, 237, 203, 250, 87, 203,
	220, 25, 226, 46, 29, 223, 85, 6, 229, 203, 1, 195, 109, 168, 181, 47, 72, 131, 127, 0, 103, 154, 
	178, 200, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 189, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 208, 0, 0, 0, 0, 163, 105, 151, 35, 150, 179, 0, 0, 0, 0, 0,
	90, 104, 85, 95, 130, 184, 81, 8, 204, 49, 170, 44, 83, 46, 0 
};

struct [[nodiscard]] MavlinkHeaderV1 final{

    // 用于检测丢包。组件对每条发送的消息进行增值。
    uint8_t seq;

    // (0, 255]发送消息的系统（载具）识别码。用于区分网络上的系统。
    // 注意，广播地址0不能在此字段中使用，因为它是一个无效的源地址。
    uint8_t sysid;

    // (0, 255] 发送消息的组件编号。用于区分系统中的各个组件（例如自动驾驶仪和摄像头）。
    // MAV_COMPONENT中使用合适的数值。请注意，广播地址不能用于该字段，因为它是一个无效的源地址。
    uint8_t compid;
};



struct [[nodiscard]] MavlinkHeaderV2 final{
    uint8_t len;
    uint8_t incompat_flags;
    uint8_t compat_flags;
    uint8_t seq;
    uint8_t sysid;
    uint8_t compid;
};





template<typename Serializer>
static constexpr Result<void, typename Serializer::Error> 
serialize_tailer(
    Serializer & srz,

    //需要参与crc计算的字节
    std::span<const uint8_t> bytes,
    uint8_t crc_extra
) noexcept{
    const auto buf = Mcrf4xxChecksumBuilder::from_default()
        .push_bytes(bytes)
        .push_byte(crc_extra)
        .finalize_to_u8x2();

    if(const auto res = srz.push_bytes(std::span(buf));
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


template<typename Serializer>
static constexpr Result<void, typename Serializer::Error> 
serialize_signature(
    Serializer & srz, 
    std::span<const uint8_t, SIGNATURE_LENGTH> signature
) noexcept{
    if(const auto res = (srz.push_bytes(signature));
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

template<typename Msg, typename Serializer>
static constexpr Result<void, typename Serializer::Error> 
serialize_msg_v1(
    Serializer & srz, 
    const MavlinkHeaderV1 & header,
    const Msg & msg
) noexcept{

    #ifndef _NDEBUG
    MAVLINK_DEBUG_ASSERT(header.sysid != 0, "sysid can't be 0")
    MAVLINK_DEBUG_ASSERT(header.compid != 0, "compid can't be 0")
    #endif

    const uint8_t msg_id = static_cast<uint8_t>(Msg::MSG_ID);
    const uint8_t len = Msg::BYTES_SIZE;
    const uint8_t crc_extra = MAVLINKV1_EXTRA_CRC_TABLE[msg_id];

    {
        const uint8_t buf[] = {
            MAVLINK_V1_HEADER,
            len,
            header.seq,
            header.sysid,
            header.compid,
            msg_id
        };
    
        
        if(const auto res = srz.push_bytes(std::span(buf));
            res.is_err()) return Err(res.unwrap_err());
    }

    if(const auto res = srz.push_bytes(msg.as_bytes());
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = serialize_tailer(srz, msg.as_bytes(), crc_extra);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

#if 0
template<typename Msg, typename Serializer>
static constexpr Result<void, typename Serializer::Error> 
serialize_msg_v2(
    Serializer & srz, 
    const MavlinkHeaderV2 & header,
    const Msg & msg
) noexcept{

    #ifndef _NDEBUG
    MAVLINK_DEBUG_ASSERT(header.sysid != 0, "sysid can't be 0")
    MAVLINK_DEBUG_ASSERT(header.compid != 0, "compid can't be 0")
    #endif

    const uint8_t msg_id = static_cast<uint8_t>(Msg::MSG_ID);
    const uint8_t crc_extra = MAVLINKV1_EXTRA_CRC_TABLE[msg_id];

    {
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
    
        
        if(const auto res = srz.push_bytes(std::span(buf));
            res.is_err()) return Err(res.unwrap_err());
    }

    if(const auto res = srz.push_bytes(msg.as_bytes());
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = serialize_tailer(srz, msg.as_bytes(), crc_extra);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}
#endif

}