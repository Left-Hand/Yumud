#include "LD19.hpp"
#include <span>

using namespace ymd;
using namespace ymd::drivers;

using Self = LD19_StreamParser;

void Self::push_byte(const uint8_t byte){
    switch(state_){
        case State::WaitingHeader:
            if(byte != HEADER_TOKEN){
                reset();
                break;
            }
            frame_.header = byte;
            bytes_count_++;
            state_ = State::WaitingVerlen;
            break;
        case State::WaitingVerlen:
            if(byte != VERLEN_TOKEN){
                reset();
                break;
            }
            frame_.verlen = byte;
            bytes_count_ ++;
            state_ = State::Remaining;
            break;
        case State::Remaining:
            bytes_[bytes_count_] = byte;
            bytes_count_++;
            if(bytes_count_ == FRAME_SIZE){
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

    const auto expected_crc = frame_.crc8;
    const auto actual_crc = frame_.calc_crc();

    if(not (expected_crc == actual_crc)) [[unlikely]] {
        callback_(Event(Events::InvalidCrc{
            .expected = expected_crc,
            .actual = actual_crc
        }));
    }

    callback_(Event(Events::FrameReady{frame_}));
}

void Self::reset(){
    bytes_count_ = 0;
    state_ = State::WaitingHeader;
}