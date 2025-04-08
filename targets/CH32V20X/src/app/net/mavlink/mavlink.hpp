#pragma once

#include <span>

#include "core/math/real.hpp"
#include "core/polymorphism/reflect.hpp"

namespace ymd::net::mavlink{

template<typename T>
class Sink;

template<typename T>
class Source;

template<>
class Sink<std::byte>{
public:
    size_t write(std::span<const std::byte> pdata);
    size_t pending() const;
};

using BytesSink = Sink<std::byte>;

template<>
class Source<std::byte>{
public:
    size_t read(std::span<std::byte> pdata);
    size_t available() const;
};

using BytesSource = Source<std::byte>;

struct MavlinkHeaderV1{
    const uint8_t header = 0xFE;
    uint8_t len;
    uint8_t seq;
    uint8_t sysid;
    uint8_t compid;
    uint8_t msgid;

    std::span<const std::byte> as_span() const{
        return std::span<const std::byte>(
            reinterpret_cast<const std::byte *>(&header), 6);
    }
};

struct MavlinkTrailerV1{
    uint8_t crc_low;
    uint8_t crc_high;

    static MavlinkTrailerV1 from_buf(std::span<const std::byte> buf){
        const auto crc = calc_crc(buf);
        const uint8_t low = crc & 0xFF;
        const uint8_t high = (crc >> 8) & 0xFF;
        return MavlinkTrailerV1{low, high};
    }

    static uint16_t calc_crc(std::span<const std::byte> buf) {
        uint16_t crc = 0xFFFF; // 初始值为 0xFFFF
        for (auto b : buf) {
            crc ^= static_cast<uint16_t>(b) << 8;
            for (int i = 0; i < 8; ++i) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ 0x1021; // 多项式 0x1021
                } else {
                    crc <<= 1;
                }
            }
        }
        return crc;
    }

    std::span<const std::byte> as_span() const{
        return std::span<const std::byte>(
            reinterpret_cast<const std::byte *>(&crc_low), 2);
    }
};

struct MavlinkFrame{
    MavlinkHeaderV1 header;
    std::span<const std::byte> payload;
    MavlinkTrailerV1 trailer;
};

BytesSink & operator << (BytesSink & sink, const MavlinkFrame & frame){
    sink.write(frame.header.as_span());
    sink.write(frame.payload);
    sink.write(frame.trailer.as_span());
    return sink;
}

template<typename TProtocol>
class Sender;

template<typename TProtocol>
class Receiver;

struct MavlinkProtocol;



template<>
class Sender<MavlinkProtocol>{
    Sender(BytesSink & sink): sink_(sink){}

    void send(const MavlinkFrame & frame){
        sink_ << frame;
    }
private:
    BytesSink & sink_;
};



template<>
class Receiver<MavlinkProtocol>{
    Receiver(BytesSource & source): source_(source){}
private:
    BytesSource & source_;
};

using MavlinkSender = Sender<MavlinkProtocol>;
using MavlinkReceiver = Receiver<MavlinkProtocol>;

}