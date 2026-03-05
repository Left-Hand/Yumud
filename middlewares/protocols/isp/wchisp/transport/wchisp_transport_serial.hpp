#pragma once

#include "core/utils/Result.hpp"

//参考
// https://github.com/ch32-rs/wchisp/blob/main/src/transport/serial.rs

namespace ymd::wchisp::transport::serial{

static constexpr std::array<uint8_t, 2> CLIENT_REQ_HEAD_BYTES = {0x57, 0xAB};
static constexpr std::array<uint8_t, 2> CLIENT_RESP_HEAD_BYTES = {0x55, 0xaa};

template<typename Receiver>
constexpr Result<void, typename Receiver::Error> 
client_write_bytes(
    Receiver& receiver, 
    const std::span<const uint8_t> bytes
){
    uint8_t crc = 0;
    for(size_t i = 0; i < bytes.size(); ++i){
        crc = static_cast<uint8_t>(crc + bytes[i]);
    }

    if(const auto res = receiver.try_push_bytes(std::span(CLIENT_REQ_HEAD_BYTES));
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = receiver.try_push_bytes(std::span(bytes));
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = receiver.try_push_bytes(std::span<const uint8_t>(&crc, 1)); 
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

template<typename Receiver>
constexpr Result<void, typename Receiver::Error> 
server_write_bytes(
    Receiver& receiver, 
    const std::span<const uint8_t> bytes
) {
    const uint8_t payload_head_bytes[4] = {
        0x00, 0x00, static_cast<uint8_t>(bytes.size()), static_cast<uint8_t>(bytes.size() >> 8)
    }; 

    //发送帧头
    if (const auto res = receiver.try_push_bytes(std::span(CLIENT_RESP_HEAD_BYTES));
        res.is_err()) {
        return Err(res.unwrap_err());
    }

    //发送 4 字节载荷头部
    if (const auto res = receiver.try_push_bytes(std::span(payload_head_bytes));
        res.is_err()) {
        return Err(res.unwrap_err());
    }

    //发送载荷数据
    if (const auto res = receiver.try_push_bytes(bytes);
        res.is_err()) {
        return Err(res.unwrap_err());
    }

    //发送校验和
    
    uint8_t crc = 0;
    for (size_t i = 0; i < 4; ++i) {
        crc += payload_head_bytes[i];
    }
    for (size_t i = 0; i < bytes.size(); ++i) {
        crc += bytes[i];
    }

    if (const auto res = receiver.try_push_bytes(std::span<const uint8_t>(&crc, 1));
        res.is_err()) {
        return Err(res.unwrap_err());
    }

    return Ok();
}


}