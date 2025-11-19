#pragma once

#include "../cyphal_crc.hpp"
#include "../cyphal_primitive.hpp"
#include <cstdint>

// 笔记
// cobs算法将uint8_t[N]映射到uint8_t[N+1]的空间（多牺牲一字节 从信息熵上满足256^N <= 255^(N+1)
// cyphal通过0分割每个帧，每个帧由首部(固定24字节)-载荷-32位CRC拼接而成，帧字节通过cobs算法映射为无0码

// This section specifies a concrete transport that operates on top of raw byte-level communication channels,
// such as TCP/IP connections, SSL, UART, RS-232, RS-422, USB CDC ACM, and any similar communication
// links that allow exchange of unstructured byte streams. Cyphal/serial may also be used to store Cyphal frames
// in files. As of this version, the Cyphal/serial specification remains experimental. Breaking changes affecting
// wire compatibility are possible.
// As Cyphal/serial is designed to operate over unstructured byte streams, it defines a custom framing protocol,
// custom frame header format, and a custom integrity checking mechanism.


namespace ymd::cyphal::transport::serial{

using namespace cyphal::primitive;


alignas(4) struct [[nodiscard]]Header{
    using Self = Header;

    //offset 0
    uint8_t version:4;
    uint8_t :4;

    //offset 1
    TransferPriority priority:3;
    uint8_t :5;

    //offset 2
    uint16_t source_node_id;

    //offset 4
    uint16_t destination_node_id;
    
    //offset 6
    uint16_t data_specifier:15;
    uint16_t service_else_message:1;
    
    //offset 8
    uint64_t transfer_id;

    //offset 16
    uint32_t frame_index:31;
    uint32_t end_of_transfer:1;

    //offset 20
    uint16_t user_data;

    //offset 22
    uint16_t header_crc16_big_endian;

    [[nodiscard]] constexpr std::span<const uint8_t, 24> as_bytes() const {
        return std::span<const uint8_t, 24>(reinterpret_cast<const uint8_t *>(this), 24);
    }

    [[nodiscard]] constexpr std::span<uint8_t, 24> as_mut_bytes() {
        return std::span<uint8_t, 24>(reinterpret_cast<uint8_t *>(this), 24);
    }

    constexpr Self from_bytes(const std::span<const uint8_t, 24> bytes){
        for(size_t i = 0; i < 24; i++){
            as_mut_bytes()[i] = bytes[i];
        }
    }

    constexpr Self from_aligned_bytes(const std::span<const uint8_t, 24> bytes){
        for(size_t i = 0; i < 24; i++){
            reinterpret_cast<size_t *>(this)[i] = reinterpret_cast<const size_t *>(bytes.data())[i];
        }
    }
    constexpr void fill_bytes(const std::span<uint8_t, 24> bytes) const {
        for(size_t i = 0; i < 24; i++){
            bytes[i] = as_bytes()[i];
        }
    }

    constexpr void fill_aligned_bytes(const std::span<uint8_t, 24> bytes) const {
        for(size_t i = 0; i < 24 / sizeof(size_t); i++){
            reinterpret_cast<size_t *>(bytes.data())[i] = reinterpret_cast<const size_t *>(this)[i];
        }
    }


    [[nodiscard]] constexpr uint8_t calc_crc() const {
        return crc::crc16(as_bytes());
    }

    [[nodiscard]] constexpr bool is_crc_valid() const {
        return __builtin_bswap16(calc_crc() == header_crc16_big_endian);
    }
};

static_assert(sizeof(Header) == 24);

// The snippet given below contains the hexadecimal dump of the following Cyphal/serial transfer:
// Priority nominal
// Transfer-ID 0
// Transfer kind message with the subject-ID 1234
// Source node-ID 1234
// Destination node-ID None
// Header user data 0
// Transfer payload uavcan.primitive.String.1 containing string “012345678”
// The payload is shown in segments for clarity:

// The first byte is the starting delimiter of the first frame.
// • The second byte is a COBS overhead byte (one for the entire transfer).
// • The following block of 24 bytes is the COBS-encoded header.
// • The third-to-last block is the COBS-encoded transfer payload, containing the two bytes of the array length
// prefix followed by the string data.
// • The second-to-last block of four bytes is the COBS-encoded transfer-CRC.
// • The last byte is the ending frame delimiter

// 00
// 09
// 01 04 d2 04 ff ff d2 04 01 01 01 01 01 01 01 01 01 01 02 80 01 04 08 12
// 09 0e 30 31 32 33 34 35 36 37 38
// 84 a2 2d e2
// 00
}