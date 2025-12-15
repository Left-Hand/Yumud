#pragma once

#include "hal/bus/uart/uart.hpp"
#include "core/math/real.hpp"
#include "core/utils/sumtype.hpp"

namespace ymd::drivers::mk8000tr{


static constexpr size_t DEFAULT_UART_BAUD = 115200;
static constexpr uint8_t HEADER_TOKEN = 0xF0;
static constexpr uint8_t LEN_TOKEN = 0x05;
static constexpr uint8_t TAIL_TOKEN = 0xAA;

#pragma pack(push, 1)
struct [[nodiscard]] SignalStrengthCode{
    using Self = SignalStrengthCode;
    uint8_t bits;

    static constexpr SignalStrengthCode from_bits(const uint8_t bits) {
        return SignalStrengthCode{bits};
    }

    template<typename T = int16_t>
    constexpr T to_dbm() const {
        return static_cast<T>(bits) - 256;
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self){
        return os << self.to_dbm() << "dBm";
    }
};

struct [[nodiscard]] Packet{
    using Self = Packet;

    uint16_t sender_addr;
    uint16_t dist_cm;
    SignalStrengthCode signal_strength;

    friend OutputStream & operator<<(OutputStream & os, const Self & self){
        return os 
            << os.field("sender_addr")(self.sender_addr) << os.splitter()
            << os.field("dist_cm")(self.dist_cm) << os.splitter()
            << os.field("signal_strength")(self.signal_strength)
        ;
    }
};
#pragma pack(pop)

static_assert(sizeof(Packet) == 5);


using Event = Packet;

using Callback = std::function<void(Event)>;


class MK8000TR_ParserSink final{
public:
    explicit MK8000TR_ParserSink(Callback callback):
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
    struct TransportFrame{
        #pragma pack(push, 1)
        uint8_t header;
        uint8_t length;
        // uint16_t sender_addr;
        // uint16_t dist_cm;
        // SignalStrengthCode signal_strength;
        Packet packet;
        uint8_t tail;
        #pragma pack(pop)

    };

    static_assert(8 == sizeof(TransportFrame));
    union{
        TransportFrame frame_;
        std::array<uint8_t, sizeof(TransportFrame)> bytes_;
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