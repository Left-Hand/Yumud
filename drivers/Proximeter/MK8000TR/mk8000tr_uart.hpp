#pragma once

#include "hal/bus/uart/uart.hpp"
#include "core/math/real.hpp"
#include "core/utils/sumtype.hpp"

namespace ymd::drivers{

struct MK8000TR_Prelude{

static constexpr uint8_t HEADER_TOKEN = 0xF0;
static constexpr uint8_t LEN_TOKEN = 0x05;
static constexpr uint8_t TAIL_TOKEN = 0xAA;

struct SignalStrength{
    uint8_t count;
    constexpr int32_t to_dbm() const {
        return static_cast<int32_t>(count) - 256;
    }
};

struct Packet{
    uint8_t header;
    uint8_t length;
    uint16_t sender_addr;
    uint16_t dist_cm;
    SignalStrength signal_strength;
    uint8_t tail;
};


using Event = Packet;

using Callback = std::function<void(Event)>;
};

class MK8000TR_StreamParser final:public MK8000TR_Prelude{
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
    union{
        Packet packet_;
        std::array<uint8_t, sizeof(Packet)> bytes_;
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