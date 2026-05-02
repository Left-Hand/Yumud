#pragma once

#include "stl06n_utils.hpp"
#include "stl06n_msgs.hpp"


namespace ymd::drivers::stl06n{




class [[nodiscard]] STL06N_ParseReceiver final{
public:
    explicit STL06N_ParseReceiver(Callback && callback);

    void push_byte(const uint8_t byte);

    void push_bytes(const std::span<const uint8_t> bytes);



    void reset();

public:

    union{
        alignas(4) std::array<uint8_t, 48> bytes_;
    };

    Callback callback_;

    struct CommandCode{
        uint8_t bits;

        constexpr Option<Command> parse() const {
            return Command::try_from_u8(bits);
        }
    };

    Option<Command> may_command_ = None;

    enum class FsmState:uint8_t{
        AwaitingHeader = 0,
        AwaitingVerlen,
        Payload,
    };


    volatile FsmState fsm_state_ = FsmState::AwaitingHeader;
    volatile bool is_emitting_ = false; 


    size_t bytes_count_;

    void flush(const Command command);
};

}
