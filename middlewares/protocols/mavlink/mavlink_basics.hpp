#pragma once


#include "mavlink_prelude.hpp"


namespace ymd::mavlink{



struct [[nodiscard]] MavlinkHeaderV1{
    const uint8_t header = 0xFE;
    uint8_t len;
    uint8_t seq;
    uint8_t sysid;
    uint8_t compid;
    uint8_t msgid;

    [[nodiscard]] std::span<const uint8_t> as_bytes() const{
        return std::span<const uint8_t>(
            reinterpret_cast<const uint8_t *>(&header), 6);
    }
};

struct [[nodiscard]] MavlinkTrailerV1{
    uint8_t crc_low;
    uint8_t crc_high;

    static constexpr MavlinkTrailerV1 from_bytes(std::span<const uint8_t> buf){
        const auto crc = calc_crc(buf);
        const uint8_t low = crc & 0xFF;
        const uint8_t high = (crc >> 8) & 0xFF;
        return MavlinkTrailerV1{low, high};
    }

    [[nodiscard]] static constexpr uint16_t calc_crc(std::span<const uint8_t> buf) {
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

    [[nodiscard]] std::span<const uint8_t> as_bytes() const{
        return std::span<const uint8_t>(
            reinterpret_cast<const uint8_t *>(&crc_low), 2);
    }
};

struct [[nodiscrad]] MavlinkFrame{
    MavlinkHeaderV1 header;
    std::span<const uint8_t> payload;
    MavlinkTrailerV1 trailer;
};

BytesSink & operator << (BytesSink & sink, const MavlinkFrame & frame){
    sink.write(frame.header.as_bytes());
    sink.write(frame.payload);
    sink.write(frame.trailer.as_bytes());
    return sink;
}



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