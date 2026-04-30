#pragma once

#include "m10_prelude.hpp"


namespace ymd::drivers::m10{

class M10_ParseReceiver final{
public:
    explicit M10_ParseReceiver(Callback callback):
        callback_(callback)
    {
        reset();
    }

    void push_byte(const uint8_t byte);

    void push_bytes(const std::span<const uint8_t> bytes){
        for(const auto byte : bytes){
            push_byte(byte);
        }
    }

    void flush();

    void reset(){
        state_ = FsmState::WaitingHeader1;
        bytes_count_ = 0;
    }
private:
    union{
        LidarSector sector_;
        alignas(4) std::array<uint8_t, sizeof(LidarSector)> bytes_;
    };

    Callback callback_;

    size_t bytes_count_ = 0;

    enum class FsmState:uint8_t{
        WaitingHeader1,
        WaitingHeader2,
        Remaining
    };

    volatile FsmState state_ = FsmState::WaitingHeader1;

};
}