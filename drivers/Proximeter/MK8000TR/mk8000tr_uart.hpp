#pragma once

#include "hal/bus/uart/uart.hpp"
#include "core/math/real.hpp"
#include "core/utils/sumtype.hpp"

namespace ymd::drivers{

struct MK8000TR_Prelude{
static constexpr size_t DEFAULT_UART_BAUD = 115200;
static constexpr uint8_t HEADER_TOKEN = 0xF0;
static constexpr uint8_t LEN_TOKEN = 0x05;
static constexpr uint8_t TAIL_TOKEN = 0xAA;

struct [[nodiscard]] SignalStrength{
    using Self = SignalStrength;

    uint8_t bits;

    static constexpr SignalStrength from_bits(const uint8_t bits) {
        return SignalStrength{bits};
    }
    constexpr int32_t to_dbm() const {
        return static_cast<int32_t>(bits) - 256;
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self){
        return os << self.to_dbm() << "dBm";
        // return os << (self.bits - 256) << "dBm";
        // return os << (self.bits ) << "dBm";
    }
};

struct [[nodiscard]] Packet{
    using Self = Packet;


    uint16_t sender_addr;
    uint16_t dist_cm;
    SignalStrength signal_strength;

    friend OutputStream & operator<<(OutputStream & os, const Self & self){
        return os << "Packet{" <<
            "sender_addr: " << self.sender_addr << os.splitter() <<
            "dist_cm: " << self.dist_cm << os.splitter() <<
            "signal_strength: " << self.signal_strength <<
            "}";
    }
};


using Event = Packet;

using Callback = std::function<void(Event)>;
};

class MK8000TR_StreamParser final:public MK8000TR_Prelude{
public:
    explicit MK8000TR_StreamParser(Callback callback):
        callback_(callback)
    {
        reset();
    }

    void push_byte(const uint8_t byte);

    void push_bytes(const std::span<const uint8_t> bytes);

    void flush();

    void reset();
private:

    Callback callback_;
    struct Frame{
        uint8_t header;
        uint8_t length;
        uint16_t sender_addr;
        uint16_t dist_cm;
        SignalStrength signal_strength;
        uint8_t tail;

        constexpr Packet to_packet() const {
            return Packet{
                .sender_addr = sender_addr,
                .dist_cm = dist_cm,
                .signal_strength = signal_strength
            };
        }
    };

    static_assert(8 == sizeof(Frame));
    union{
        Frame frame_;
        std::array<uint8_t, sizeof(Frame)> bytes_;
    };
    size_t bytes_count_ = 0;

    enum class State:uint8_t{
        WaitingHeader,
        WaitingLen,
        Remaining
    };

    State state_ = State::WaitingHeader;

};

}