#pragma once

#include "stl06n_utils.hpp"


namespace ymd::drivers::stl06n{


using Callback = std::function<void(Event)>;


class STL06N_ParseReceiver final{
public:
    explicit STL06N_ParseReceiver(Callback callback):
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


    Command command_ = Command::Sector;

    enum class FsmState:uint8_t{
        WaitingHeader,
        WaitingVerlen,
        Remaining,
        Emitting,
    };

    size_t bytes_count_ = 0;
    volatile FsmState fsm_state_ = FsmState::WaitingHeader;

};

}