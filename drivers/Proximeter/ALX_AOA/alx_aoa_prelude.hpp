#pragma once

#include "hal/bus/uart/uart.hpp"
#include "core/math/real.hpp"
#include "core/utils/sumtype.hpp"
#include "core/container/heapless_vector.hpp"

#include "primitive/arithmetic/angular.hpp"
#include "algebra/vectors/spherical_coordinates.hpp"

namespace ymd::drivers::alx_aoa{



static constexpr uint32_t DEFAULT_UART_BAUD = 115200;
static constexpr uint16_t PROTOCOL_VERSION = 0x0100;
static constexpr size_t LEADER_SIZE = 6; //4 bytes(header) + 2 bytes(len)
enum class [[nodiscard]] Error:uint8_t{
    InvalidCommand,
    InvalidProtocolVersion,
    InvalidTagStatus,
    InvalidXor,
    InvalidRequest,
    InvalidLength
};

OutputStream & operator <<(OutputStream & os, const Error & error);

enum class [[nodiscard]] TargetStatus:uint16_t{
    Normal = 0x1234
};

enum class [[nodiscard]] RequestCommand:uint16_t{
    Location = 0x2001,
    HeartBeat = 0x2002,
};

struct [[nodiscard]] TargetDistanceCode final{
    using Self = TargetDistanceCode;
    uint32_t bits;

    template<typename T = uq16> 
    [[nodiscard]] constexpr T to_meters() const{
        return T(bits) / 100;
    }
    static constexpr Self from_bits(const uint32_t bits){
        return Self{bits};
    }
    friend OutputStream & operator <<(OutputStream & os, const TargetDistanceCode & self){ 
        return os << self.to_meters() << "m";
    }
};

struct [[nodiscard]] TargetAngleCode final{
    using Self = TargetAngleCode;
    int16_t bits;

    template<typename T = uq16>
    [[nodiscard]] constexpr Angular<T> to_angle() const{
        return Angular<T>::from_degrees(bits);
    }
    static constexpr Self from_bits(const int16_t bits){
        return Self{bits};
    }
    friend OutputStream & operator <<(OutputStream & os, const TargetAngleCode & self){ 
        return os << self.to_angle().to_degrees() << "deg";
    }
};

struct [[nodiscard]] DeviceIdCode final{
    using Self = DeviceIdCode;
    uint32_t bits;

    static constexpr Self from_bits(const uint32_t bits){
        return Self{bits};
    }
    friend OutputStream & operator <<(OutputStream & os, const DeviceIdCode & self){ 
        auto guard = os.create_guard();
        return os << std::showbase << std::hex << self.bits;
    }
};


struct [[nodiscard]] Location final{
    DeviceIdCode anchor_id;
    DeviceIdCode target_id;
    TargetDistanceCode distance;
    TargetAngleCode azimuth;
    TargetAngleCode elevation;

    template<typename T>
    constexpr SphericalCoordinates<T> to_spherical_coordinates() const{
        return SphericalCoordinates<T>{
            distance.to_meters<T>(),
            azimuth.to_angle<T>(), 
            elevation.to_angle<T>() 
        };
    };

    friend OutputStream & operator <<(OutputStream & os, const Location & self){
        return os
            << os.field("anchor_id")(self.anchor_id) << os.splitter()
            << os.field("target_id")(self.target_id) << os.splitter()
            << os.field("distance")(self.distance) << os.splitter()
            << os.field("azimuth")(self.azimuth) << os.splitter()
            << os.field("elevation")(self.elevation)
        ;
    }
};

struct [[nodiscard]] HeartBeat final{
    DeviceIdCode anchor_id;

    friend OutputStream & operator <<(OutputStream & os, const HeartBeat & self){ 
        return os << "heartbeat" << self.anchor_id;
    }
};

struct [[nodiscard]] Event:public Sumtype<std::monostate, HeartBeat, Location>{};
using Callback = std::function<void(Result<Event, Error>)>;

struct [[nodiscard]] LeaderInfo final{
    uint8_t len;
    uint16_t command;
};

class AlxAoa_ParseReceiver final{
public:
    enum class FsmState:uint8_t{
        Header0,
        Header1,
        Header2,
        Header3,
        WaitingLen0,
        WaitingLen1,
        Remaining
    };

    explicit AlxAoa_ParseReceiver(Callback callback):
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
        fsm_state_ = FsmState::Header0;
    }
private:
    static constexpr size_t MAX_PAYLOAD_SIZE = 32;


    friend OutputStream & operator <<(OutputStream & os, const AlxAoa_ParseReceiver::FsmState & prog);

    Result<Event, Error> parse();

    HeaplessVector<uint8_t, MAX_PAYLOAD_SIZE> payload_bytes_ = {};
    
    volatile FsmState fsm_state_ = FsmState::Header0;
    Callback callback_ = nullptr;

    LeaderInfo leader_info_ = {};

};

}