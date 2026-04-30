#pragma once

#include "alx_aoa_prelude.hpp"

namespace ymd::drivers::alx_aoa{

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

    HeaderInfo header_info_ = {};

};

}