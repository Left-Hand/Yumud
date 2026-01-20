#include "m10_prelude.hpp"

using namespace ymd::drivers::m10;

void M10_ParseReceiver::push_byte(const uint8_t byte){
    switch(state_){
        case FsmState::WaitingHeader1:
            if(byte != HEADER1_TOKEN){reset(); break;}
            state_ = FsmState::WaitingHeader2;
            break;
        case FsmState::WaitingHeader2:
            if(byte != HEADER2_TOKEN){reset(); break;}
            state_ = FsmState::Remaining;
            break;
        case FsmState::Remaining:
            bytes_[bytes_count_] = byte;
            bytes_count_ +=1;
            if(bytes_count_ >= NUM_SECTOR_BYTES){
                flush();
                reset();
            }
            break;
    
    }
}


void M10_ParseReceiver::flush(){
    if(callback_ == nullptr) [[unlikely]]
        __builtin_trap();
    if(bytes_count_ != NUM_SECTOR_BYTES) [[unlikely]]
        __builtin_trap();
    const auto event = Event(events::DataReady{sector_});
    callback_(event);
}