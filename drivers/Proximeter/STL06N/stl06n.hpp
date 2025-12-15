#pragma once

#include "stl06n_utils.hpp"


namespace ymd::drivers::stl06n{


using Callback = std::function<void(Event)>;


class STL06N_ParserSink final{
public:
    explicit STL06N_ParserSink(Callback callback):
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
        alignas(4) std::array<uint8_t, 48> bytes_;
    };
    size_t bytes_count_ = 0;
    Command command_ = Command::Sector;

    enum class State:uint8_t{
        WaitingHeader,
        WaitingVerlen,
        Remaining,
        Emitting,
    };

    State state_ = State::WaitingHeader;

};

}