#pragma once

#include "stl06n_utils.hpp"


namespace ymd::drivers::stl06n{


using Callback = std::function<void(Event)>;


class [[nodiscard]] STL06N_ParseReceiver final{
public:
    explicit STL06N_ParseReceiver(Callback && callback);

    void push_byte(const uint8_t byte);

    void push_bytes(const std::span<const uint8_t> bytes);

    [[nodiscard]] bool is_emitting() const{
        return fsm_state_ == FsmState::Emitting;
    }

    void flush();

    void reset();
// private:
public:

    union{
        alignas(4) std::array<uint8_t, 48> bytes_;
    };

    Callback callback_;

    Option<Command> may_command_ = None;

    enum class FsmState:uint8_t{
        AwaitingHeader = 0,
        AwaitingVerlen,
        Remaining,
        Emitting,
    };

    size_t bytes_count_;
    volatile FsmState fsm_state_;

};

}
