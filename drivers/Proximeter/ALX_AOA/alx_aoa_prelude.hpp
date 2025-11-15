#pragma once

#include "hal/bus/uart/uart.hpp"
#include "core/math/real.hpp"
#include "core/utils/sumtype.hpp"
#include "core/utils/angle.hpp"
#include "core/utils/bytes_spawner.hpp"
#include "core/container/inline_vector.hpp"

namespace ymd::drivers{


struct AlxAoa_Prelude{

static constexpr uint32_t DEFAULT_UART_BUAD = 115200;
static constexpr uint16_t PROTOCOL_VERSION = 0x0100;
static constexpr size_t LEADER_SIZE = 6; //4 bytes(header) + 2 bytes(len)
enum class Error:uint8_t{
    InvalidCommand,
    InvalidProtocolVersion,
    InvalidTagStatus,
    InvalidXor,
    InvalidRequest,
    InvalidLength
};

friend OutputStream & operator <<(OutputStream & os, const AlxAoa_Prelude::Error & error);

enum class TargetStatus:uint16_t{
    Normal = 0x1234
};

enum class RequestCommand:uint16_t{
    Location = 0x2001,
    HeartBeat = 0x2002,
};

struct [[nodiscard]] TargetDistanceCode{
    uint32_t bits;
    [[nodiscard]] constexpr uq16 to_meters() const{
        return uq16(bits) / 100;
    }

    friend OutputStream & operator <<(OutputStream & os, const TargetDistanceCode & self){ 
        return os << self.to_meters() << "m";
    }
};

struct [[nodiscard]] TargetAngleCode{
    int16_t bits;

    [[nodiscard]] constexpr Angle<uq16> to_angle() const{
        return Angle<uq16>::from_degrees(bits);
    }

    friend OutputStream & operator <<(OutputStream & os, const TargetAngleCode & self){ 
        return os << self.to_angle().to_degrees() << "deg";
    }
};

struct [[nodiscard]] DeviceIdCode{
    uint32_t bits;


    friend OutputStream & operator <<(OutputStream & os, const DeviceIdCode & self){ 
        auto guard = os.create_guard();
        return os << std::showbase << std::hex << self.bits;
    }
};


struct [[nodiscard]] Location{
    DeviceIdCode anchor_id;
    DeviceIdCode target_id;
    TargetDistanceCode distance;
    TargetAngleCode azimuth;
    TargetAngleCode elevation;

    friend OutputStream & operator <<(OutputStream & os, const Location & self){
        return os << "location" << self.anchor_id << "->" << self.target_id << 
            ": " << self.distance << " " << self.azimuth << " " << self.elevation;
    }
};

struct [[nodiscard]] HeartBeat{
    DeviceIdCode anchor_id;

    friend OutputStream & operator <<(OutputStream & os, const HeartBeat & self){ 
        return os << "heartbeat" << self.anchor_id;
    }
};

struct Event:public Sumtype<std::monostate, HeartBeat, Location>{};
using Callback = std::function<void(Result<Event, Error>)>;

struct LeaderInfo{
    uint8_t len;
    uint16_t command;
};



};

class AlxAoa_StreamParser final:public AlxAoa_Prelude{
public:
    explicit AlxAoa_StreamParser(Callback callback):
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

    friend OutputStream & operator <<(OutputStream & os, const AlxAoa_StreamParser::ByteProg & prog);

    Result<Event, Error> parse();

    Callback callback_ = nullptr;
    ByteProg byte_prog_ = ByteProg::Header0;
    LeaderInfo leader_info_ = {};
    HeaplessVector<uint8_t, MAX_PAYLOAD_SIZE> payload_bytes_ = {};

};

}