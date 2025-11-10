#pragma once

#include "hal/bus/uart/uart.hpp"
#include "core/math/real.hpp"
#include "core/utils/sumtype.hpp"
#include "core/utils/angle.hpp"
#include "core/container/inline_vector.hpp"

namespace ymd::drivers{

struct [[nodiscard]] BytesSpawner{
    explicit constexpr BytesSpawner(std::span<const uint8_t> bytes) : 
        bytes_(bytes) {}

    template<size_t N>
    [[nodiscard]] constexpr std::span<const uint8_t, N> spawn(){
        // if(bytes_.size() < N) __builtin_abort();
        const auto ret = std::span<const uint8_t, N>(bytes_.data(), N);
        bytes_ = std::span<const uint8_t>(bytes_.data() + N, bytes_.size() - N);
        return ret;
    }

    [[nodiscard]] constexpr std::span<const uint8_t> remaining() const {
        return bytes_;
    }
private:
    std::span<const uint8_t> bytes_;
};

struct ALXAOA_Prelude{


static constexpr uint16_t PROTOCOL_VERSION = 0x0100;
static constexpr size_t LEADER_SIZE = 6; //4 bytes(header) + 2 bytes(len)
enum class Error:uint8_t{
    InvalidCommand,
    InvalidProtocolVersion,
    InvalidTagStatus,
    InvalidCrc,
    InvalidRequest
};

enum class TargetStatus:uint16_t{
    Normal
};

enum class RequestCommand:uint16_t{
    Location = 0x2001,
    HeartBeat = 0x2002,
};

struct TargetDistance{
    uint16_t distance_cm;

    [[nodiscard]] constexpr uq16 to_meters() const{
        return uq16(distance_cm) / 100;
    }
};

struct TargetAngle{
    int8_t degrees;
    [[nodiscard]] constexpr Angle<uq16> to_angle() const{
        return Angle<uq16>::from_degrees(degrees);
    }
};

struct DeviceId{
    uint32_t count;
};


struct Location{
    DeviceId anchor_id;
    DeviceId target_id;
    TargetDistance distance;
    TargetAngle azimuth;
    TargetAngle elevation;
};

struct HeartBeat{
    DeviceId anchor_id;
};

struct LeaderInfo{
    uint16_t len;
    uint16_t command;
};

using Event = Sumtype<Location, HeartBeat>;
using Callback = std::function<void(Result<Event, Error>)>;
};

class ALXAOA_StreamParser final:public ALXAOA_Prelude{
    explicit ALXAOA_StreamParser(Callback callback):
        callback_(callback)
    {
        reset();
    }

    void push_bytes(const std::span<const uint8_t> bytes){
        for(const auto byte : bytes){
            push_byte(byte);
        }
    }


    void push_byte(const uint8_t byte);
    void flush();
    void reset(){
        payload_bytes_.clear();
        byte_prog_ = ByteProg::Header0;
    }
private:
    static constexpr size_t MAX_PAYLOAD_SIZE = 32;

    enum class ByteProg:uint8_t{
        Header0,
        Header1,
        Header2,
        Header3,
        WaitingLen0,
        WaitingLen1,
        Remaining
    };

    Callback callback_ = nullptr;
    ByteProg byte_prog_ = ByteProg::Header0;
    LeaderInfo leader_info_ = {};
    HeaplessVector<uint8_t, MAX_PAYLOAD_SIZE> payload_bytes_ = {};

};

}