#include "stl06n.hpp"
#include <span>

using namespace ymd;
using namespace ymd::drivers::stl06n;

using Self = STL06N_ParserSink;



void Self::push_byte(const uint8_t byte){
    switch(state_){
        case State::WaitingHeader:
            if(byte != HEADER_TOKEN){
                reset();
                break;
            }
            state_ = State::WaitingVerlen;
            break;
        case State::WaitingVerlen:
            if(const auto may_command = Command::try_from_u8(byte); may_command.is_none()){
                reset();
                break;
            }else{
                command_ = may_command.unwrap();
            }
            state_ = State::Remaining;
            
            break;
        case State::Remaining:
            bytes_[bytes_count_] = byte;
            bytes_count_++;

            if(bytes_count_ >= command_.payload_size()){
                flush();
                reset();
            }
            break;
        case State::Emitting:
            PANIC{"racing condition is happening!!!"};
            break;
    }
}

void Self::push_bytes(const std::span<const uint8_t> bytes){ 
    for(const auto byte: bytes){
        push_byte(byte);
    }
}

#define ASSUME(expr) ({\
    const bool is_right = bool(expr);\
    if(!is_right) __builtin_unreachable();\
})\

void Self::flush(){
    if(callback_ == nullptr) [[unlikely]]
        PANIC{"callback is null"};

    state_ = State::Emitting;

    const auto num_bytes = command_.payload_size();

    const uint8_t * begin = bytes_.data();
    const uint8_t * end = begin + num_bytes;

    //尾元素本身指向crc8校验
    const uint8_t expected_crc = *end;

    const uint8_t actual_crc = [&]{
        Crc8Calculator calc;
        calc = calc.push_bytes(std::span(begin, end));
        return calc.value();
    }();

    if(expected_crc != actual_crc) [[unlikely]]{
        const auto event = Event(Event::InvalidCrc{
            .command = command_,
            .expected = expected_crc,
            .actual = actual_crc
        });
        callback_(event);
    }

    switch(command_.kind()){
        case Command::Sector:{
            const auto & sector = *reinterpret_cast<const LidarSectorPacket *>(begin);
            const auto event = Event(Event::DataReady{.sector = sector});
            callback_(event);
        }
            break;
        case Command::Start:
            callback_(Event(Event::Start{}));
            break;
        case Command::Stop:
            callback_(Event(Event::Stop{}));
            break;
        case Command::SetSpeed:
            callback_(Event(Event::SetSpeed{}));
            break;
        case Command::GetSpeed:{
            //begin[0] 为数据长度字段 固定为4
            const auto speed = LidarSpinSpeedCode::from_bytes(
                begin[1], begin[2]
            );
            callback_(Event(Event::GetSpeed{.speed = speed}));
            break;
        }
    }
    return;
}

void Self::reset(){
    bytes_count_ = 0;
    state_ = State::WaitingHeader;
    command_ = Command::Sector;
}