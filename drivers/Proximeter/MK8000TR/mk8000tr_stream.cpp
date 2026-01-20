#include "mk8000tr_stream.hpp"
#include <span>

using namespace ymd;
using namespace ymd::drivers;

namespace ymd::drivers::mk8000tr{

using Self = MK8000TR_ParseReceiver;

void Self::push_byte(const uint8_t byte){
    switch(state_){
        case FsmState::WaitingHeader:
            if(byte != HEADER_TOKEN){
                reset();
                break;
            }
            frame_.header = byte;
            bytes_count_++;
            state_ = FsmState::WaitingLen;
            break;
        case FsmState::WaitingLen:
            if(byte != LEN_TOKEN){
                reset();
                break;
            }
            frame_.length = byte;
            bytes_count_ ++;
            state_ = FsmState::Remaining;
            break;
        case FsmState::Remaining:
            bytes_[bytes_count_] = byte;
            bytes_count_++;
            if(bytes_count_ == sizeof(TransportFrame)){
                if(frame_.tail == TAIL_TOKEN)
                    flush();
                reset();
            }
            break;
    }
}

void Self::push_bytes(const std::span<const uint8_t> bytes){ 
    for(const auto byte: bytes){
        push_byte(byte);
    }
}

void Self::flush(){
    if(callback_ == nullptr) [[unlikely]]
        PANIC{"callback is null"};

    callback_(frame_.packet);
}

void Self::reset(){
    bytes_count_ = 0;
    state_ = FsmState::WaitingHeader;
}

}